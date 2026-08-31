#ifndef SESSION_H
#define SESSION_H

#include <cstring>
#include <chrono>
#include <type_traits>
#include "Connection.h"
#include "FixTags.h"
#include "MWMRNoOverWriteSlotRingBuffer.h"
#include "TypeDef.h"
#include "FixMessage.h"
#include "Events.h"
#include "CommonUtils.h"
#include "MessageBuilder.h"
#include "MessageParser.h"
#include "LogMessage.h"

class NewFixMessageEvent : public EventBase
{
public:
    NewFixMessageEvent(const FixMsgType& message) : EventBase(EventType::NEW_FIX_MESSAGE), message_(&message) {}
    const FixMsgType& getMessage() const { return *message_; }
private:
    const FixMsgType* message_ = nullptr;
};

class ConnectionCloseEvent : public EventBase
{
public:
    enum Reason { RING_BUFFER_OVERFLOW, MESSAGE_OVER_SIZE, CONNECTION_ERROR, MESSAGE_PARSE_ERROR, MANY_GAPS, BROKER_LOGOUT, FAILED_TO_ADD_OUT_QUEUE};
public:
    ConnectionCloseEvent(Reason reason) : EventBase(EventType::BROKER_CONNECTION_CLOSED) , reason_(reason) { }
    Reason getReason() { return reason_; }
    Reason reason_;
};

class LogonSuccessEvent : public EventBase
{
public:
    LogonSuccessEvent() : EventBase(EventType::LOGON_SUCCESS) {}
};

class RawFixMessage
{
public:
    enum Status {VALID, RECIEVING, NOT_VALID};
public:
    RawFixMessage(std::size_t arrSize = 1024) {
        if (!isPowerOfTwo(arrSize)) {
            throw std::invalid_argument("Size ahould be power of two");
        }
        message_.reserve(arrSize);
    }
public:
    std::size_t getArraySize() { return message_.capacity(); }
    char* getMessage() { return message_.data(); }
    const char* getMessage() const { return message_.data(); }
    void setStatus(Status st) { st_ = st; }
    Status getSatus() const { return st_; }
    void setMessageSize(std::size_t size) { msgSize_ = size; }
    std::size_t getMessageSize() const { return msgSize_; }
    void setMessageSeqNum(std::size_t seqNum) { msgSeqNum_ = seqNum; }
    std::size_t getMessageSeqNum() const { return msgSeqNum_; }
private:
    std::vector<char> message_;
    Status st_ = NOT_VALID;
    std::size_t msgSize_;
    std::size_t msgSeqNum_ = 0;
};

 class SessionConfig
{
public:
    SessionConfig(std::string_view id, std::string_view targetId, std::string_view host, int port) : id_(id), targetId_(targetId), host_(host), port_(port) {}
    void setUserName(std::string_view userName) { userName_ = userName; }
    void setPassWord(std::string_view passWord) { passWord_ = passWord; }
    void setHeartBeatInterval(TimeStamp interval) { heartBeatInterval_ = interval; }
    void setIpvType(Connection::IpvType type) { ipvType_ = type; }
    void setOutMessageBufferSize(std::size_t size) { outMessageBufferSize_  = size; }
    void setOutQueueSize(std::size_t size) { outQueueSize_ = size; }
    void setSentQueueSize(std::size_t size) { sentQueueSize_ =  size; }
    void setSocketReadBufferSize(std::size_t size) { socketReadBufferSize_ = size; }
    void setDataInRingBufferSize(std::size_t size) { dataInRingBufferSize_ = size; }
    void setGapMsgQueueSize(std::size_t size) { gapMsgQueueSize_ = size; }
    void setGapMsgBufferSize(std::size_t size) { gapMsgBufferSize_ = size; }
    std::string getID() const { return id_; }
    std::string getTargetId() const { return targetId_; }
    std::string getHost() const { return host_; }
    int getPort() const { return port_; }
    std::string getUserName() const { return userName_; }
    std::string getPassWord() const { return passWord_; }
    TimeStamp getHeartBeatInterval() const { return heartBeatInterval_; }
    Connection::IpvType getIpvType() const { return ipvType_; }
    std::size_t getOutMessageBufferSize() const { return outMessageBufferSize_; }
    std::size_t getOutQueueSize() const { return outQueueSize_; }
    std::size_t getSentQueueSize() const { return sentQueueSize_; }
    std::size_t getSocketReadBufferSize() const { return socketReadBufferSize_; }
    std::size_t getDataInRingBufferSize() const { return dataInRingBufferSize_; }
    std::size_t getGapMsgQueueSize() const { return gapMsgQueueSize_; }
    std::size_t getGapMsgBufferSize() const { return gapMsgBufferSize_; } 
private:
    std::string id_;
    std::string targetId_;
    std::string host_;
    int port_;
    std::string userName_;
    std::string passWord_;
    TimeStamp heartBeatInterval_ = 30;
    Connection::IpvType ipvType_ = Connection::IpvType::IPV4;
    std::size_t outMessageBufferSize_ = 2048;
    std::size_t outQueueSize_ = 256;
    std::size_t sentQueueSize_ = 256;
    std::size_t socketReadBufferSize_ = 4096;
    std::size_t dataInRingBufferSize_ = 8192;
    std::size_t gapMsgQueueSize_ = 128;
    std::size_t gapMsgBufferSize_ = 1024;
};

enum class SessionStatus { CONNECTING, CONNECTED, LOGGEDIN, LOGGEDOUT, DISCONNECTED };

template<typename MsgParser, typename MsgBuilder, typename Logger> class Session
{

public:
    using OutMessageQueue =  MWMRNoOverWriteSlotRingBuffer<OutMessage>;
public:
    Session(const SessionConfig& config, MsgParser messageParser, MsgBuilder messageBuilder, Logger& logger);
    const ConnectionID& getID() const { return conn_.getID(); }
    bool addMessageToSend(const FixLogonMessage& msg) { return addMessageToSendTmp(msg);}
    bool addMessageToSend(const FixHeartBeatMessage& msg) { return addMessageToSendTmp(msg);}
    bool addMessageToSend(const FixMarketDataRequest& msg) { return addMessageToSendTmp(msg);}
    bool openSession();
    void closeImidietely(ConnectionCloseEvent::Reason reason);
    SessionStatus checkStatus();
    bool isSessionReady();
    void readMessages();
    void sendMessages();
    void checkAndSendHeartBeat();
    void registerForSessionEvents(Subscriber subscriber) const  {  subscribers_.push_back(subscriber); }
    void registerForEvents(Subscriber subscriber) const  {  subscribers_.push_back(subscriber); }
    Logger& getLogger() { return logger_; }
    SessionIDType getSessionID() { return sessionID_; }
private:
    template<typename T> bool addMessageToSendTmp(const T& message);
    void handleNewMessage(const FixMsgType& msg);  
    bool appendToRingBuffer(char* array, int size);
    std::size_t getNextMsgSeqNum() { return ++msgSeqNum_; }
    void notifySubscribers(const EventBase& event) const;
    bool copyFromRingBuffer(char *array, std::size_t length) const;
    void updateRecieveTimePoint() { lastRecieved_ = std::chrono::steady_clock::now();}
    void resendReuest(std::size_t startIndex, std::size_t endIndex);
    void printFromRingBuffer(std::ostream& os, char *array, std::size_t length ) const;
private:
    MsgParser messageParser_;
    MsgBuilder messageBuilder_;
    Logger& logger_;  
    std::string id_;
    std::string targetId_;   
    Connection conn_;
    std::string userName_;
    std::string passWord_;
    OutMessageQueue outQueue_;
    std::vector<OutMessage> sentQueue_;
    std::vector<char> bufferIn_;
    mutable std::vector<Subscriber> subscribers_;
    std::vector<char> ringBuffer_;
    std::size_t ringBufferSize_;
    std::size_t mask_;
    std::size_t start_ = 0;
    std::size_t parseStart_ = 0;
    std::size_t end_ = 0;
    std::size_t msgSeqNum_ = 0;
    std::size_t expIncomingMsgSeqNum_ = 1;
    std::size_t latestIncomingMsgSeqNum_ = 0;
    std::size_t gapMsgQueueSize_;
    std::chrono::seconds heartBeat_;
    std::chrono::seconds destinationHeartBeat_ = std::chrono::seconds(30);
    std::chrono::steady_clock::time_point lastSentTime_;
    std::chrono::steady_clock::time_point lastRecieved_;
    char testString_[50];
    char testStringLength_ = 0;
    std::vector<RawFixMessage> gapMsgQueue_;
    OutMessageQueue::Slot* currentOutMessageSlot_ = nullptr;
    SessionStatus sessionStatus_ =  SessionStatus::DISCONNECTED;
    SessionIDType sessionID_;
    static SessionIDType sessionIDCounter_;
};

template<typename MsgParser, typename MsgBuilder, typename Logger>  SessionIDType Session<MsgParser, MsgBuilder, Logger>::sessionIDCounter_ = 0;

template<typename MsgParser, typename MsgBuilder, typename Logger> Session<MsgParser, MsgBuilder, Logger>::Session(const SessionConfig& config, MsgParser messageParser, MsgBuilder messageBuilder, Logger& logger):
                                         messageParser_(messageParser), messageBuilder_(std::move(messageBuilder)), logger_(logger), id_(config.getID()), targetId_(config.getTargetId()),
                                         conn_(config.getHost(), config.getPort(), config.getIpvType()), userName_(config.getUserName()), passWord_(config.getPassWord()),
                                         outQueue_(config.getOutQueueSize(), OutMessage(config.getOutMessageBufferSize())), sentQueue_(config.getSentQueueSize(), OutMessage(config.getOutMessageBufferSize())),
                                         bufferIn_(config.getSocketReadBufferSize()), ringBuffer_(config.getDataInRingBufferSize()), ringBufferSize_(config.getDataInRingBufferSize()),
                                         mask_(ringBufferSize_ -1), gapMsgQueueSize_(config.getGapMsgQueueSize()), heartBeat_(config.getHeartBeatInterval()),
                                         gapMsgQueue_(config.getGapMsgQueueSize(), RawFixMessage(config.getGapMsgQueueSize()))
{
    subscribers_.reserve(1);
    sessionID_ = ++sessionIDCounter_;
    SessionCreated sessionCreated(sessionID_, config.getPort(), config.getHost());
    logger_.logMessage(sessionCreated);
}

template<typename MsgParser, typename MsgBuilder, typename Logger> template<typename T>
bool Session<MsgParser, MsgBuilder, Logger>::addMessageToSendTmp(const T& message)
{
    auto slotPtr = outQueue_.getWriteSlot();

    if (!slotPtr) {
        OutgoingSlotUnavailable log(sessionID_);
        logger_.logMessage(log);
        return false;
    }

    auto &outMessage = slotPtr->getData();
    outMessage.reset();

    bool dataAdded = messageBuilder_.addDataToOutMsg(message, outMessage, id_, targetId_);

    if (!dataAdded) {
        MessageBuildingFailed log(sessionID_);
        logger_.logMessage(log);
        this->closeImidietely(ConnectionCloseEvent::FAILED_TO_ADD_OUT_QUEUE);
        return false;
    }

    outQueue_.setWriteComplete(slotPtr);
    return true;

}

template<typename MsgParser, typename MsgBuilder, typename Logger> void Session<MsgParser, MsgBuilder, Logger>::sendMessages()
{
    while (isSessionReady()) {

        if (!currentOutMessageSlot_) {
            currentOutMessageSlot_ =  outQueue_.getReadSlot();

            if (!currentOutMessageSlot_) {
                return;
            }

            auto &outMessage = currentOutMessageSlot_->getData();
            messageBuilder_.finalizeOutMessage(outMessage, getNextMsgSeqNum());

        }

        auto &outMessage = currentOutMessageSlot_->getData();
        std::cout << "sending message " << outMessage << std::endl;
        int st = conn_.send(outMessage.buffer_.data() + outMessage.sentCount_, outMessage.dataSize_ - outMessage.sentCount_);

        if (st > 0) {
            outMessage.sentCount_ += st;
            lastSentTime_ = std::chrono::steady_clock::now();

            if (outMessage.sentCount_ == outMessage.dataSize_) {
                outQueue_.setReadComplete(currentOutMessageSlot_);
                currentOutMessageSlot_ = nullptr;
            }

        } else if ((errno != EWOULDBLOCK) && (errno != EAGAIN)) {
            closeImidietely(ConnectionCloseEvent::Reason::MESSAGE_OVER_SIZE);
        }

    }

}

template<typename MsgParser, typename MsgBuilder, typename Logger> bool Session<MsgParser, MsgBuilder, Logger>::openSession()
{
    auto st = conn_.connect();

    if (st == Connection::Status::CONNECTING) {
        SessionConnecting constatus(sessionID_);
        logger_.logMessage(constatus);
    } else if (st == Connection::Status::CONNECTED) {
        sessionStatus_ = SessionStatus::CONNECTED;
        SessionConnected connStatus(sessionID_);
        logger_.logMessage(connStatus);
    } else {
        SessionDisconnected connStatus(sessionID_);
        logger_.logMessage(connStatus);
        return false;
    }

    FixLogonMessage msg;
    msg.setResetSeqNumFlag(ResetSeqNumFlag::Types::YES);
    msg.setHeartBeatInterval(heartBeat_.count());
    msg.setUserName(userName_);
    msg.setPassWord(passWord_);

    if (!addMessageToSend(msg)) {
        LogonMessageSendingFailed log(sessionID_);
        logger_.logMessage(log);
        return false;
    }

    return true;
}

template<typename MsgParser, typename MsgBuilder, typename Logger> SessionStatus Session<MsgParser, MsgBuilder, Logger>::checkStatus()
{
    if (sessionStatus_ == SessionStatus::CONNECTING) {
        auto st = conn_.checkStatus();

        if (st == Connection::Status::CONNECTING) {
            sessionStatus_ = SessionStatus::CONNECTING;
        } else if (st == Connection::Status::CONNECTED) {
            sessionStatus_ = SessionStatus::CONNECTED;
        }
    }
    return sessionStatus_;
}

template<typename MsgParser, typename MsgBuilder, typename Logger> bool Session<MsgParser, MsgBuilder, Logger>::isSessionReady()
{
    auto status = checkStatus();
    return (status == SessionStatus::CONNECTED || status == SessionStatus::LOGGEDIN);
}

template<typename MsgParser, typename MsgBuilder, typename Logger> void Session<MsgParser, MsgBuilder, Logger>::readMessages()
{

    while (isSessionReady()) {
        auto numBytes = conn_.receive(bufferIn_.data(), bufferIn_.size());

        if (numBytes > 0) {

            if (!appendToRingBuffer(bufferIn_.data(), numBytes)) {
                InputRingBufferOverflow log(sessionID_);
                logger_.logMessage(log);
                closeImidietely(ConnectionCloseEvent::Reason::RING_BUFFER_OVERFLOW);
                return;
            }

            const ParseStatus &sth = messageParser_.parseHeader(ringBuffer_.data(), parseStart_, end_, mask_);
            auto typeh = sth.getType();

            if (typeh == ParseStatus::Type::SUCCESS) {
                updateRecieveTimePoint();
                auto& successH = static_cast<const ParseSuccess&>(sth);
                auto &msgH = static_cast<const FixMessageHeader&>(successH.getMessage());
                auto seqNum = msgH.getMessageSeqNum();
                auto msgLength = msgH.getTotalMsgSize();
                
                
                if (seqNum > latestIncomingMsgSeqNum_)
                    latestIncomingMsgSeqNum_ = seqNum;

                if (seqNum == expIncomingMsgSeqNum_) {

                    const ParseStatus &stb = messageParser_.parseBody();
                    auto typeb = stb.getType();

                    if (typeb == ParseStatus::Type::SUCCESS) {
                        auto& successB = static_cast<const ParseSuccess&>(stb);
                        auto &msgH = successH.getMessage();
                        handleNewMessage(msgH);
                        ++expIncomingMsgSeqNum_;
                    } else if (typeb == ParseStatus::Type::TAG_READ_ERROR) {
                        MessageParseError log(sessionID_);
                        logger_.logMessage(log);
                        closeImidietely(ConnectionCloseEvent::Reason::MESSAGE_PARSE_ERROR);
                        return;
                    }

                } else if (seqNum > expIncomingMsgSeqNum_) {

                    auto gapBufferIndex = seqNum % gapMsgQueueSize_;
                    auto& gapBufferEntry = gapMsgQueue_[gapBufferIndex];
                    auto msgArraySize = gapBufferEntry.getArraySize();
                    auto gspSeqNum = gapBufferEntry.getMessageSeqNum();
                    auto status= gapBufferEntry.getSatus();

                    if ((status != RawFixMessage::Status::NOT_VALID)) {
                        HighMissingMessageCount log(sessionID_);
                        closeImidietely(ConnectionCloseEvent::Reason::MANY_GAPS);
                        return;
                    }

                    if (msgLength > msgArraySize) {
                        InputRingBufferOverflow log(sessionID_);
                        logger_.logMessage(log);
                        closeImidietely(ConnectionCloseEvent::Reason::MESSAGE_OVER_SIZE);
                        return;
                    } 

                    char* msgArray = gapBufferEntry.getMessage();
                    copyFromRingBuffer(msgArray, msgH.getTotalMsgSize());
                    gapBufferEntry.setMessageSeqNum(seqNum);
                    gapBufferEntry.setStatus(RawFixMessage::Status::VALID);
                }

                start_ += msgLength;
                parseStart_ += msgLength;

            } else if (typeh == ParseStatus::Type::MESSAGE_NOT_COMPLETE) {
                continue;
            } else {
                MessageParseError log(sessionID_);
                logger_.logMessage(log);
                closeImidietely(ConnectionCloseEvent::Reason::MESSAGE_PARSE_ERROR);
                return;
            }

        } else if ((numBytes == 0) || ((numBytes < 0) && (errno != EWOULDBLOCK) && (errno != EAGAIN))) {
            closeImidietely(ConnectionCloseEvent::Reason::CONNECTION_ERROR);
            return;
        } else {
            break;
        }
    }

    std::size_t count = 0;

    for (auto i = expIncomingMsgSeqNum_ ; i <= latestIncomingMsgSeqNum_; ++i) {
        auto gapBufferIndex = i % gapMsgQueueSize_;
        auto& gapBufferEntry = gapMsgQueue_[gapBufferIndex];
        auto seqNum = gapBufferEntry.getMessageSeqNum();
        auto status = gapBufferEntry.getSatus();

        if (status != RawFixMessage::Status::NOT_VALID) {
            break;
        }
        gapBufferEntry.setMessageSeqNum(i);
        gapBufferEntry.setStatus(RawFixMessage::Status::RECIEVING);
        ++count;
    }

    for (auto i = expIncomingMsgSeqNum_ ; i <= latestIncomingMsgSeqNum_; ++i) {
        auto gapBufferIndex = i % gapMsgQueueSize_;
        auto& gapBufferEntry = gapMsgQueue_[gapBufferIndex];
        auto seqNum = gapBufferEntry.getMessageSeqNum();
        auto status = gapBufferEntry.getSatus();

        if ((seqNum != i ) || (status != RawFixMessage::Status::VALID)) {
            break;
        }
        
        messageParser_.parseHeader(gapBufferEntry.getMessage(), 0, gapBufferEntry.getMessageSize(), gapBufferEntry.getMessageSize() - 1);
        const ParseStatus &stb = messageParser_.parseBody();
        auto typeb = stb.getType();

        if (typeb == ParseStatus::Type::SUCCESS) {
            auto& successB = static_cast<const ParseSuccess&>(stb);
            auto &msgB = successB.getMessage();
            handleNewMessage(msgB);
            ++expIncomingMsgSeqNum_;
        } else if (typeb == ParseStatus::Type::TAG_READ_ERROR) {
            MessageParseError log(sessionID_);
            logger_.logMessage(log);
            closeImidietely(ConnectionCloseEvent::Reason::MESSAGE_PARSE_ERROR);
            return;
        }

        gapBufferEntry.setStatus(RawFixMessage::Status::NOT_VALID);
    }

}

template<typename MsgParser, typename MsgBuilder, typename Logger> void Session<MsgParser, MsgBuilder, Logger>::handleNewMessage(const FixMsgType& msg)
{
    auto type = msg.getMessageType();

    if (type == FixMessageType::LOGON) {
        auto &loginMsg = static_cast<const FixLogonMessage&>(msg);
        destinationHeartBeat_ = std::chrono::seconds(loginMsg.getHeartBeatInterval());
        sessionStatus_ = SessionStatus::LOGGEDIN;
        LogonSuccess log(sessionID_);
        logger_.logMessage(log);
        notifySubscribers(LogonSuccessEvent());
    } else if (type == FixMessageType::LOGOUT) {
        sessionStatus_ = SessionStatus::LOGGEDOUT;
        LogonFailed log(sessionID_);
        logger_.logMessage(log);
        closeImidietely(ConnectionCloseEvent::BROKER_LOGOUT);
        return;
    } else if (type == FixMessageType ::HEART_BEAT) {
        auto& htBtmsg = static_cast<const FixHeartBeatMessage&>(msg);
        auto testId = htBtmsg.getTestID();
        strcpy(testString_, testId); 
    } else {
        NewFixMessageEvent event(msg);
        notifySubscribers(event);
    }
}

template<typename MsgParser, typename MsgBuilder, typename Logger> void Session<MsgParser, MsgBuilder, Logger>::closeImidietely(ConnectionCloseEvent::Reason reason)
{
    conn_.disconnect();
    sessionStatus_ =  SessionStatus::DISCONNECTED;
    ImidiateSessionClosed log(sessionID_);
    logger_.logMessage(log);
    notifySubscribers(ConnectionCloseEvent(reason));
}

template<typename MsgParser, typename MsgBuilder, typename Logger> void Session<MsgParser, MsgBuilder, Logger>::notifySubscribers(const EventBase& event) const
{
    for (auto& subscriber : subscribers_) {
        subscriber.notify(event);
    }
}

template<typename MsgParser, typename MsgBuilder, typename Logger> bool Session<MsgParser, MsgBuilder, Logger>::appendToRingBuffer(char* array, int size) 
{
    auto endIndex_ = end_ & mask_;
   std::size_t remain = ringBufferSize_ - endIndex_;

   if (remain >= size) {
        memcpy(&ringBuffer_[endIndex_], array, size);
        end_ += size;
   } else {
        memcpy(&ringBuffer_[endIndex_], array, remain);
        auto remainBytesToWrite = size  - remain;
        memcpy(&ringBuffer_[0], array + remain, remainBytesToWrite);
        end_ += size;

        if (end_ - start_ > ringBufferSize_)
            return false;

        start_ += remainBytesToWrite;
   }

   return true;
}

template<typename MsgParser, typename MsgBuilder, typename Logger> bool Session<MsgParser, MsgBuilder, Logger>::copyFromRingBuffer(char *array, std::size_t length) const
{
    auto startIndex_ = start_ & mask_;

   if (startIndex_ + length <= ringBufferSize_) {
        memcpy(array, &ringBuffer_[startIndex_], length);
   } else {
        auto l1 = ringBufferSize_ - startIndex_;
        memcpy(array, &ringBuffer_[startIndex_], l1);
        memcpy(array + l1, &ringBuffer_[0], length - l1);
   }

   return true;
};

template<typename MsgParser, typename MsgBuilder, typename Logger> void Session<MsgParser, MsgBuilder, Logger>::checkAndSendHeartBeat()
{
    auto duration = std::chrono::steady_clock::now() - lastSentTime_;
    auto durationS = std::chrono::duration_cast<std::chrono::seconds>(duration);

    if (durationS < destinationHeartBeat_)
        return;

    FixHeartBeatMessage heartBtMsg;
    auto testId = heartBtMsg.getTestID();
    strcpy(testId, testString_);

    if (!addMessageToSend(heartBtMsg)) {
        return;
    }

    sendMessages();
}

template<typename MsgParser, typename MsgBuilder, typename Logger> void Session<MsgParser, MsgBuilder, Logger>::printFromRingBuffer(std::ostream& os, char *array, std::size_t length ) const
{
    for (auto i = 0; i <= length; ++i) {
        char c = ringBuffer_[(i + start_) & mask_];

        if (c == SOH)
            c = '|';

        os << c;
    }
}

template<typename MsgParser, typename MsgBuilder, typename Logger> void Session<MsgParser, MsgBuilder, Logger>::resendReuest(std::size_t startIndex, std::size_t endIndex)
{

}

#endif