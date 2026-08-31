#ifndef BINARY_LOGGER_H
#define BINARY_LOGGER_H

#include <string>
#include <vector>
#include <deque>
#include <thread>
#include <memory>
#include <type_traits>
#include "MWMRNoOverWriteSlotRingBuffer.h"
#include "LogMessage.h"
#include "BinaryMessageReader.h"

class BinaryLogFileWriter
{
public:
    BinaryLogFileWriter(const BinaryLogFileWriter& config) = delete;
    BinaryLogFileWriter& operator=(const BinaryLogFileWriter& config) = delete;
    BinaryLogFileWriter(std::string_view filePrefix, std::size_t maxSize, std::size_t batchWriteSize);
public:
    bool write(const BinaryLogMessage& msg);
    bool closeFile();
    bool init();
private:
    std::string_view getNextFileName();
    std::string_view getFileName() { return std::string_view(fileName_.data()); }
    bool openFile();

private:
    int fd_ = -1;
    int fileNumPos_ = 0;
    std::string_view filePrefix_;
    std::size_t maxFileSize_ = 0;
    std::size_t currentFileSize_ = 0;
    std::size_t nextSeqNo_ = 0;
    std::size_t currentBufferSize_ = 0;
    std::vector<char> fileName_;
    std::vector<char> buffer_;
    BinaryLogMessage fileEndMessage_;
};

class BinaryLogFileReader
{
    friend class BinaryLogger;
public:
    ~BinaryLogFileReader();
    std::pair<void*, LogMessageType> getNextMessage();
    bool openFile();
    void closeFile();
private:
    BinaryLogFileReader(std::string_view fileName, const std::vector<std::unique_ptr<BinaryMessageReader>> &fileReaderList);
    BinaryLogFileReader(const BinaryLogFileReader&) = delete;
    BinaryLogFileReader& operator=(const BinaryLogFileReader&) = delete;
private:
    std::vector<std::unique_ptr<BinaryMessageReader>> messageReaderList_;
    std::string fileName_;
    int fd_ = -1;
    char* filePtr_ = nullptr;
    char* readPtr_ = nullptr;
    bool readComplete_ = false;
    char readBuffer_[BinaryLogMessage::maxDataSize_];
    std::size_t fileSize_ = 0;
};

class BinaryLogger
{
public:
    BinaryLogger(std::string_view filePrefix, std::size_t msgQueueSize = 10000);
    ~BinaryLogger();
    template<ValidLogMessage T> void registerLogMessage(LogMessageType type);
    void setLogDiectory(std::string_view logDirectory) { logDirectory_ = logDirectory; }
    template<ValidLogMessage T> bool logMessage(const T& message);
    bool start();
    void stop();
    std::unique_ptr<BinaryLogFileReader> getLogFileReader(std::string_view fileName);
    void setFileMaxSizeBytes(std::size_t maxSize) { maxSize_ = maxSize; }
    void setBatchWriteSizeBytes(std::size_t size) {batchWriteSize_= size; }

    std::string_view getFilePrefix() const { return filePrefix_;  }
    std::size_t getFileMaxSizeBytes() const { return maxSize_; }
    std::size_t getBatchWriteSizeBytes() const { return batchWriteSize_; }
private:
    bool init();
    void exec();
private:
    std::string filePrefix_;
    std::size_t maxSize_ = 102400; //100MB
    std::size_t batchWriteSize_ = 102400; //10KB
    std::unique_ptr<BinaryLogFileWriter> fileWriter_;
    std::vector<std::unique_ptr<BinaryMessageReader>> messageReaderList_;
    std::string logDirectory_;
    std::size_t msgQueueSize_;
    MWMRNoOverWriteSlotRingBuffer<BinaryLogMessage> messageQueue_;
    std::jthread writerThread_;
    std::atomic<std::size_t> logCount_;               
    bool stop_ = false;
};

template<ValidLogMessage T> void  BinaryLogger::registerLogMessage(LogMessageType type)
{
    messageReaderList_[static_cast<int>(type)] = std::make_unique<BinaryMessageReaderImpl<T>>();
}

template<ValidLogMessage T> bool BinaryLogger::logMessage(const T& message)
{
    if (!messageReaderList_[static_cast<int>(message.getType())])
        return false;

    auto slot = messageQueue_.getWriteSlot();

    if (!slot)
        return false;

    auto &data = slot->getData();
    convertToBinaryLogMessage(message, data);
    messageQueue_.setWriteComplete(slot);
    logCount_.fetch_add(1, std::memory_order_acq_rel);
    logCount_.notify_one();
    return true;
}

template<ValidLogMessage T> inline void convertToBinaryLogMessage(const T& msg, BinaryLogMessage &logMsg)
{
    logMsg.dataSize_ = sizeof(msg);
    memcpy(logMsg.data_, &msg, sizeof(msg));
}

template<> inline void convertToBinaryLogMessage<StringLog>(const StringLog& msg, BinaryLogMessage &logMsg)
{
    auto type = msg.getType();
    auto length = msg.getMessageSize();
    logMsg.dataSize_ = sizeof(type) + sizeof(length) + length;
    auto charrArray = logMsg.data_;
    memcpy(charrArray, &type, sizeof(type));
    charrArray += sizeof(type);
    memcpy(charrArray, &length, sizeof(length));
    charrArray += sizeof(length);
    memcpy(charrArray, msg.getMessage().data(), length);
}


#endif