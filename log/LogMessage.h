#ifndef LOG_MESSAGE_H
#define LOG_MESSAGE_H

#include <string>
#include <cstring>
#include <cstdint>
#include <memory>
 
enum class MessageType : uint32_t
{
    CONNECTION_SUCCESS_1 = 0,
    FILE_END_1,
    STRING_MESSAGE,
    COUNT
};

using MessageLengthType = std::size_t;

class ConnectionSuccess1
{
public:
    void setHostName(std::string_view hostName);
    void setPort(int port) { port_ = port; }
    MessageType getType() const { return type_; }
    std::string_view getHostName() const { return std::string_view(hostName_); }
    int getPort() const { return port_; }
private:
    MessageType type_ = MessageType::CONNECTION_SUCCESS_1;
    char hostName_[128];
    int32_t port_;

};

class FileEnd1
{
public:
    MessageType getType() const { return type_; }
    static FileEnd1 make();
private:
    MessageType type_ = MessageType::FILE_END_1;
};

template<typename T> class BinaryMessageReaderImpl;

class StringMessage
{
    friend  class BinaryMessageReaderImpl<StringMessage>;
public:
    void setMessage(std::string_view msg);
    MessageType getType() const { return type_; }
    MessageLengthType getMessageSize() const { return msgSize_; }
    std::string_view getMessage() const { return std::string_view(message_, msgSize_); }
private:
    MessageType type_ = MessageType::STRING_MESSAGE;
    MessageLengthType msgSize_ = 0;
    char message_[1012];
};

#endif