#include <filesystem>
#include "BinaryLogger.h"
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

bool writeFull(int fd, char *buffer, size_t size) {
    auto totalWrittenBytes = 0;

    while (totalWrittenBytes < size) {
        auto written =  ::write(fd + totalWrittenBytes, buffer , size - totalWrittenBytes);

        if (written < 0 && (written != EINTR)) {
            return false;
        }
        
        totalWrittenBytes += written;
    }
    return true;
}

BinaryLogger::BinaryLogger(std::string_view filePrefix ,std::size_t msgQueueSize) : filePrefix_(filePrefix),
                   messageReaderList_(static_cast<int>(LogMessageType::COUNT)),
                   msgQueueSize_(msgQueueSize),
                   messageQueue_(msgQueueSize) {
                   logCount_.store(0);
                   }

bool BinaryLogger::init()
{
   if (!logDirectory_.empty()) {
        std::filesystem::path dir(logDirectory_);
        bool dirExit = std::filesystem::exists(dir) && std::filesystem::is_directory(dir);

        if (!dirExit) {
            std::cout << "Log directory " << logDirectory_ << " does not exist.";

            try {
                if (!std::filesystem::create_directories(logDirectory_)) {
                    std::cout << "Could not create log directory." << std::endl ;
                    return false;
                } else {
                    std::cout << "Log directory created" << std::endl;
                }
            } catch (const std::filesystem::filesystem_error& e) {
                std::cout << e.what() << std::endl;
                return false;        
            }
        } else {
            std::cout << "Log directory " << logDirectory_ << " exist. Files may be overwritten.";
        } 
    }
    
    fileWriter_.reset(new BinaryLogFileWriter(filePrefix_, maxSize_, batchWriteSize_));
    
    if (!fileWriter_->init())
        return false;

    registerLogMessage<FileEndLog>(LogMessageType::FILE_END);
    registerLogMessage<StringLog>(LogMessageType::STRING_MESSAGE);
    registerLogMessage<SessionCreated>(LogMessageType::SESSIION_CREATED);
    registerLogMessage<SessionConnected>(LogMessageType::SESSION_CONNECTED);
    registerLogMessage<SessionConnecting>(LogMessageType::SESSION_CONNECTING);
    registerLogMessage<SessionDisconnected>(LogMessageType::SESSION_DISCONNECTED);
    registerLogMessage<OutgoingSlotUnavailable>(LogMessageType::OUTGOING_SLOT_UNAVAILABLE);
    registerLogMessage<MessageBuildingFailed>(LogMessageType::MESSAGE_BUILDING_FAILED);
    registerLogMessage<ImidiateSessionClosed>(LogMessageType::IMIDIATE_SESSION_CLOSED);
    registerLogMessage<LogonSuccess>(LogMessageType::LOGON_SUCCESS);
    registerLogMessage<LogonFailed>(LogMessageType::LOGON_FAILED);
    registerLogMessage<LogonMessageSendingFailed>(LogMessageType::LOGON_MESSAGE_SEINDING_FAILED);
    registerLogMessage<InputRingBufferOverflow>(LogMessageType::INPUT_RING_BUFFER_OVERFLOW);
    registerLogMessage<MessageParseError>(LogMessageType::MESSAGE_PARSE_ERROR);
    registerLogMessage<HighMissingMessageCount>(LogMessageType::HIGH_MISSING_MESSAGE_COUNT);
    registerLogMessage<InputMessageDoesNotFitInGapBuffer>(LogMessageType::INPUT_MESSAGE_DOES_NOT_FIT_IN_GAP_BUFFER);
    registerLogMessage<MarketDataRequestSendingFailed>(LogMessageType::MARKET_DATA_REUEST_SENDING_FAILED);

  
    return true;
}

bool BinaryLogger::start()
{
    if (!init())
        return false;

    writerThread_ = std::jthread(&BinaryLogger::exec, this);
    std::cout << "Log manager started" << std::endl;
    return true;
}

void BinaryLogger::exec()
{
    BinaryLogMessage msg;
    while (!stop_) {
        auto logCount = logCount_.load(std::memory_order_acquire);
        auto *slot = messageQueue_.getReadSlot();

        while (!slot) {
            logCount_.wait(logCount, std::memory_order_acquire);

            if (stop_) {
                fileWriter_->closeFile();
                return;
            }

            slot = messageQueue_.getReadSlot();
        }

        msg  = slot->getData();
        messageQueue_.setReadComplete(slot);
        fileWriter_->write(msg);
    }
    fileWriter_->closeFile();
}

void BinaryLogger::stop()
{
    stop_ = true;

    logCount_.fetch_add(1, std::memory_order_acq_rel);
    logCount_.notify_one();

    if (writerThread_.joinable())
        writerThread_.join();

    std::cout << "Log manager stopped" << std::endl;
    return;
}

std::unique_ptr<BinaryLogFileReader> BinaryLogger::getLogFileReader(std::string_view fileName)
{
    auto reader = std::unique_ptr<BinaryLogFileReader>(new BinaryLogFileReader(fileName, messageReaderList_));

    if (!reader->openFile())
        return nullptr;

    return reader;

}

BinaryLogger::~BinaryLogger()
{

}

BinaryLogFileWriter::BinaryLogFileWriter(std::string_view filePrefix, std::size_t maxSize, std::size_t batchWriteSize) : filePrefix_(filePrefix), 
                                                                maxFileSize_(maxSize),
                                                                fileName_(filePrefix.size()+25),
                                                                buffer_(batchWriteSize)
{

    
}

bool BinaryLogFileWriter::init()
{
    fileNumPos_ = snprintf(fileName_.data(), fileName_.size(),
                            "%s_", filePrefix_.data());
    if (fileNumPos_ < 0) {
        std::cout << "Constructing BinaryLogFileWriter for " << filePrefix_ << " failed" << std::endl;
        return false;
    }

    convertToBinaryLogMessage(FileEndLog(), fileEndMessage_);
    return openFile();
}

std::string_view BinaryLogFileWriter::getNextFileName()
{
    char *fileNumberPos = fileName_.data() + fileNumPos_;
    auto written = snprintf(fileNumberPos , fileName_.size() - fileNumPos_,
                            "%zu.bin", ++nextSeqNo_);
    
    if (written < 0) {
        std::cout << "Generating next file name for " << filePrefix_ << " failed." << std::endl;
        return std::string_view();
    }

    return std::string_view(fileName_.data());
    
}

bool BinaryLogFileWriter::openFile()
{
    std::string_view nextLogFIle = getNextFileName();

    if (nextLogFIle.empty()) {
        return false;
    }

    fd_ = ::open(nextLogFIle.data(), O_RDWR| O_CREAT | O_TRUNC, 0644);

    if (fd_ < 0) {
        std::cout << "Log file " << nextLogFIle << " can not be opened." << std::endl;
        return false;
    }

    auto status = fallocate(fd_, 0 , 0, maxFileSize_);

    if (status < 0) {
        std::cout << "Space allocation for file " << nextLogFIle << " failed." << std::endl;
        ::close(fd_);
        fd_ = -1;
        return false;
    }

    return true;
}

bool BinaryLogFileWriter::write(const BinaryLogMessage& msg)
{
    auto newBufferSize = currentBufferSize_ + msg.dataSize_ + sizeof(msg.dataSize_);
    auto bufferOverflow = newBufferSize > buffer_.size();
    auto fileSizeReached = currentFileSize_ + newBufferSize + sizeof(FileEndLog) + sizeof(msg.dataSize_) > maxFileSize_;

    if (bufferOverflow || fileSizeReached) {

        auto written = writeFull(fd_ , buffer_.data(), currentBufferSize_);
        
        if (!written) {
            std::cout << "Writing to log file " << getFileName() << " Failed";
            return false;
        }

        currentFileSize_ += buffer_.size();

        if (fileSizeReached) {

            if (!closeFile())
                return false;

            if (!openFile())
                return false;

            currentFileSize_ = 0;
        }

        currentBufferSize_ = 0;

    }

    memcpy(buffer_.data() + currentBufferSize_, &msg.dataSize_, sizeof(msg.dataSize_));
    currentBufferSize_ += sizeof(msg.dataSize_);
    memcpy(buffer_.data() + currentBufferSize_, msg.data_, msg.dataSize_);
    currentBufferSize_ += msg.dataSize_;
    return true;
}


bool BinaryLogFileWriter::closeFile()
{
    if (fd_ < 0)
        return false;

    if (currentBufferSize_ > 0) {
        auto written = writeFull(fd_ , buffer_.data(), currentBufferSize_);
        
        if (!written) {
            std::cout << "Writing to log file " << getFileName() << " Failed";
            return false;
        }
    }

    auto written = ::write(fd_, &fileEndMessage_.dataSize_, sizeof(fileEndMessage_.dataSize_));

    if (written < 0) {
        std::cout << "Writing end message to log file " << getFileName() << " Failed";
        return false;
    }

    written = ::write(fd_, fileEndMessage_.data_, fileEndMessage_.dataSize_);

    if (written < 0) {
        std::cout << "Writing end message to log file " << getFileName() << " Failed";
        return false;
    }

    ::close(fd_);
    fd_ = -1;
    return true;
}

BinaryLogFileReader::BinaryLogFileReader(std::string_view fileName, const std::vector<std::unique_ptr<BinaryMessageReader>> &messageReaderList): 
messageReaderList_(messageReaderList.size()), fileName_(fileName)
{
    for (std::size_t i = 0; i < messageReaderList.size(); ++i) {
        if (messageReaderList[i])
            messageReaderList_[i] =  messageReaderList[i]->clone();
    }
}


bool BinaryLogFileReader::openFile()
{
    fd_ = ::open(fileName_.c_str(), O_RDONLY);

    if (fd_ < 0)
        std::cout << "Could not open log file " << fileName_ << std::endl;

    struct stat st;

    if (fstat(fd_, &st) == -1) {
        return false;
    }

    filePtr_ = (char*) mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd_, 0);

    if (filePtr_ == MAP_FAILED) {
        return false;
    }

    readPtr_ = filePtr_;
    fileSize_ = st.st_size;

    return true;

}

std::pair<void*,LogMessageType> BinaryLogFileReader::getNextMessage()
{
    if (readComplete_)
        return std::make_pair(nullptr, LogMessageType::FILE_END);

    MessageLengthType msgLength;
    memcpy(&msgLength, readPtr_, sizeof(msgLength));

    readPtr_ += sizeof(msgLength);

    LogMessageType messageType;
    memcpy(&messageType, readPtr_, sizeof(messageType));

    if (messageType == LogMessageType::FILE_END) {
        readComplete_ = true;
        return std::make_pair(nullptr, LogMessageType::FILE_END);;
    }

    auto &reader = messageReaderList_[static_cast<std::size_t>(messageType)];
    auto message = reader->getMesssage(readPtr_, msgLength);
    
    readPtr_ += msgLength;

    return std::make_pair(message,messageType);
}

void BinaryLogFileReader::closeFile()
{
    if (fd_ != -1) {
        ::close(fd_);
        fd_ = -1;
    }

}

BinaryLogFileReader::~BinaryLogFileReader()
{

}