#ifndef LOG_MESSAGE_H
#define LOG_MESSAGE_H

#include <string>
#include <cstring>
#include <cstdint>
#include <memory>
#include <chrono>
#include "TypeDef.h"
 
enum class LogMessageType : uint32_t
{
    SESSIION_CREATED,
    SESSION_CONNECTED,
    SESSION_CONNECTING,
    SESSION_DISCONNECTED,
    IMIDIATE_SESSION_CLOSED,
    OUTGOING_SLOT_UNAVAILABLE,
    MESSAGE_BUILDING_FAILED,
    LOGON_SUCCESS,
    LOGON_FAILED,
    LOGON_MESSAGE_SEINDING_FAILED,
    INPUT_RING_BUFFER_OVERFLOW,
    MESSAGE_PARSE_ERROR,
    HIGH_MISSING_MESSAGE_COUNT,
    INPUT_MESSAGE_DOES_NOT_FIT_IN_GAP_BUFFER,
    MARKET_DATA_REUEST_SENDING_FAILED,
    STRING_MESSAGE,
    FILE_END,
    COUNT
};

using MessageLengthType = std::size_t;

inline auto getCurrentTimeStamp() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

class SessionCreated
{
public:
    SessionCreated(SessionIDType id, PortType port, std::string_view hostName);
    LogMessageType getType() const { return type_; }
    TimeStamp getTimeStamp() const { return time_; }
    SessionIDType getSessionID() const { return id_; }
    PortType getPort() const { return port_; }
    std::string_view getHostName() const { return std::string_view(hostName_); }
private:
    LogMessageType type_ = LogMessageType::SESSIION_CREATED;
    TimeStamp time_;
    SessionIDType id_;
    PortType port_;
    char hostName_[HostNameMaxSize];
};

class SessionConnected
{
public:
    SessionConnected(SessionIDType id) : time_(getCurrentTimeStamp()), id_(id) {}
    LogMessageType getType() const { return type_; }
    TimeStamp getTimeStamp() const { return time_; }
    SessionIDType getSessionID() const { return id_; }
private:
    LogMessageType type_ = LogMessageType::SESSION_CONNECTED;
    TimeStamp time_;
    SessionIDType id_;
};

class SessionConnecting
{
public:
    SessionConnecting(SessionIDType id) : time_(getCurrentTimeStamp()), id_(id) {}
    LogMessageType getType() const { return type_; }
    TimeStamp getTimeStamp() const { return time_; }
    SessionIDType getSessionID() const { return id_; }
private:
    LogMessageType type_ = LogMessageType::SESSION_CONNECTING;
    TimeStamp time_;
    SessionIDType id_;
};

class SessionDisconnected
{
public:
    SessionDisconnected(SessionIDType id) : time_(getCurrentTimeStamp()), id_(id) {}
    LogMessageType getType() const { return type_; }
    TimeStamp getTimeStamp() const { return time_; }
    SessionIDType getSessionID() const { return id_; }
private:
    LogMessageType type_ = LogMessageType::SESSION_DISCONNECTED;
    TimeStamp time_;
    SessionIDType id_;
};

class OutgoingSlotUnavailable
{
public:
    OutgoingSlotUnavailable(SessionIDType id) : time_(getCurrentTimeStamp()), id_(id) {}
    LogMessageType getType() const { return type_; }
    TimeStamp getTimeStamp() const { return time_; }
    SessionIDType getSessionID() const { return id_; }
private:
    LogMessageType type_ = LogMessageType::OUTGOING_SLOT_UNAVAILABLE;
    TimeStamp time_;
    SessionIDType id_;
};

class MessageBuildingFailed
{
public:
    MessageBuildingFailed(SessionIDType id) : time_(getCurrentTimeStamp()), id_(id) {}
    LogMessageType getType() const { return type_; }
    TimeStamp getTimeStamp() const { return time_; }    
    SessionIDType getSessionID() const { return id_; }
private:
    LogMessageType type_ = LogMessageType::MESSAGE_BUILDING_FAILED;
    TimeStamp time_;
    SessionIDType id_;
};

class ImidiateSessionClosed
{
public:
    ImidiateSessionClosed(SessionIDType id) : time_(getCurrentTimeStamp()), id_(id) {}
    LogMessageType getType() const { return type_; }
    TimeStamp getTimeStamp() const { return time_; }
    SessionIDType getSessionID() const { return id_; }
private:
    LogMessageType type_ = LogMessageType::IMIDIATE_SESSION_CLOSED;
    TimeStamp time_;
    SessionIDType id_;
};

class LogonSuccess
{
public:
    LogonSuccess(SessionIDType id) : time_(getCurrentTimeStamp()), id_(id) {}
    LogMessageType getType() const { return type_; }
    TimeStamp getTimeStamp() const { return time_; }
    SessionIDType getSessionID() const { return id_; }
private:
    LogMessageType type_ = LogMessageType::LOGON_SUCCESS;
    TimeStamp time_;
    SessionIDType id_;
};

class LogonFailed
{
public:
    LogonFailed(SessionIDType id) : time_(getCurrentTimeStamp()), id_(id) {}
    LogMessageType getType() const { return type_; }
    TimeStamp getTimeStamp() const { return time_; }
    SessionIDType getSessionID() const { return id_; }
private:
    LogMessageType type_ = LogMessageType::LOGON_FAILED;
    TimeStamp time_;
    SessionIDType id_;
};

class LogonMessageSendingFailed
{
public:
    LogonMessageSendingFailed(SessionIDType id) : time_(getCurrentTimeStamp()), id_(id) {}
    LogMessageType getType() const { return type_; }
    TimeStamp getTimeStamp() const { return time_; }
    SessionIDType getSessionID() const { return id_; }
private:
    LogMessageType type_ = LogMessageType::LOGON_MESSAGE_SEINDING_FAILED;
    TimeStamp time_;
    SessionIDType id_;
};

class InputRingBufferOverflow
{
public:
    InputRingBufferOverflow(SessionIDType id) : time_(getCurrentTimeStamp()), id_(id) {}
    LogMessageType getType() const { return type_; }
    TimeStamp getTimeStamp() const { return time_; }
    SessionIDType getSessionID() const { return id_; }
private:
    LogMessageType type_ = LogMessageType::INPUT_RING_BUFFER_OVERFLOW;
    TimeStamp time_;
    SessionIDType id_;
};

class MessageParseError
{
public:
    MessageParseError(SessionIDType id) : time_(getCurrentTimeStamp()), id_(id) {}
    LogMessageType getType() const { return type_; }
    TimeStamp getTimeStamp() const { return time_; }
    SessionIDType getSessionID() const { return id_; }
private:
    LogMessageType type_ = LogMessageType::MESSAGE_PARSE_ERROR;
    TimeStamp time_;
    SessionIDType id_;
};

class HighMissingMessageCount
{
public:
    HighMissingMessageCount(SessionIDType id) : time_(getCurrentTimeStamp()), id_(id) {}
    LogMessageType getType() const { return type_; }
    TimeStamp getTimeStamp() const { return time_; }
    SessionIDType getSessionID() const { return id_; }
private:
    LogMessageType type_ = LogMessageType::HIGH_MISSING_MESSAGE_COUNT;
    TimeStamp time_;
    SessionIDType id_;
};

class InputMessageDoesNotFitInGapBuffer
{
public:
    InputMessageDoesNotFitInGapBuffer(SessionIDType id) : time_(getCurrentTimeStamp()), id_(id) {}
    LogMessageType getType() const { return type_; }
    TimeStamp getTimeStamp() const { return time_; }
    SessionIDType getSessionID() const { return id_; }
private:
    LogMessageType type_ = LogMessageType::INPUT_MESSAGE_DOES_NOT_FIT_IN_GAP_BUFFER;
    TimeStamp time_;
    SessionIDType id_;
};

class MarketDataRequestSendingFailed
{
public:
    MarketDataRequestSendingFailed(SessionIDType id) : time_(getCurrentTimeStamp()), id_(id) {}
    LogMessageType getType() const { return type_; }
    TimeStamp getTimeStamp() const { return time_; }
    SessionIDType getSessionID() const { return id_; }
private:
    LogMessageType type_ = LogMessageType::MARKET_DATA_REUEST_SENDING_FAILED;
    TimeStamp time_;
    SessionIDType id_;
};

class FileEndLog
{
public:
    LogMessageType getType() const { return type_; }
    static FileEndLog make();
private:
    LogMessageType type_ = LogMessageType::FILE_END;
};

template<typename T> class BinaryMessageReaderImpl;

class StringLog
{
    friend  class BinaryMessageReaderImpl<StringLog>;
public:
    void setMessage(std::string_view msg);
    LogMessageType getType() const { return type_; }
    MessageLengthType getMessageSize() const { return msgSize_; }
    std::string_view getMessage() const { return std::string_view(message_, msgSize_); }
private:
    LogMessageType type_ = LogMessageType::STRING_MESSAGE;
    MessageLengthType msgSize_ = 0;
    char message_[1012];
};

#endif