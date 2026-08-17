#ifndef BINARY_MESSAGE_READER_H
#define BINARY_MESSAGE_READER_H

#include "LogMessage.h"

struct BinaryLogMessage
{
    constexpr static MessageLengthType maxDataSize_ = 1024;
    MessageLengthType dataSize_ = 0;
    char data_[maxDataSize_];

};

template<typename T> concept ValidLogMessage = requires {
    validateMessage(std::declval<T>());
};

template<typename T> constexpr bool validateMessage(const T &t)
{
   return (sizeof(T) <= BinaryLogMessage::maxDataSize_ &&
    std::is_trivial_v<T> &&
    std::is_standard_layout_v<T> &&
    offsetof(T, type_) == 0 &&
    std::is_same_v<decltype(t.type_), MessageType>);
}

class BinaryMessageReader 
{
protected:
    BinaryMessageReader() {}
    BinaryMessageReader(const BinaryMessageReader&) = delete;
    BinaryMessageReader& operator=(const BinaryMessageReader&) = delete;
public: 
    virtual ~BinaryMessageReader(){}
public:
    virtual void* getMesssage(char *memory, MessageLengthType length) const = 0;
    virtual std::unique_ptr<BinaryMessageReader> clone() const = 0;
};

template<typename T> class BinaryMessageReaderImpl : public BinaryMessageReader
{
public:
    BinaryMessageReaderImpl() : message_(new T()) { }
    BinaryMessageReaderImpl(const BinaryMessageReaderImpl&) = delete;
    BinaryMessageReaderImpl& operator=(const BinaryMessageReaderImpl&) = delete;
public:
    virtual ~BinaryMessageReaderImpl(){}

    virtual std::unique_ptr<BinaryMessageReader> clone() const override 
    {

        return std::unique_ptr<BinaryMessageReader>(new  BinaryMessageReaderImpl<T>());
    }

    virtual void* getMesssage(char *memory, MessageLengthType length) const override {
        std::memcpy(message_.get(), memory, sizeof(length));
        return message_.get();
    }
private:
    std::unique_ptr<T> message_;
};

template<> class BinaryMessageReaderImpl<StringMessage> : public BinaryMessageReader
{
public:
    BinaryMessageReaderImpl() : message_(new StringMessage()) { }
    BinaryMessageReaderImpl(const BinaryMessageReaderImpl&) = delete;
    BinaryMessageReaderImpl& operator=(const BinaryMessageReaderImpl&) = delete;
public:
    virtual ~BinaryMessageReaderImpl(){}

    virtual std::unique_ptr<BinaryMessageReader> clone() const override 
    {

        return std::unique_ptr<BinaryMessageReader>(new  BinaryMessageReaderImpl<StringMessage>());
    }

    virtual void* getMesssage(char *memory, MessageLengthType length) const override
    {
        std::memcpy(&message_->type_ , memory, sizeof(message_->type_));
        memory += sizeof(message_->type_);
        std::memcpy(&message_->msgSize_ , memory, sizeof(message_->msgSize_));
        memory += sizeof(message_->msgSize_);
        std::memcpy(message_->message_, memory, message_->msgSize_);
        return message_.get();
    }
private:
    std::unique_ptr<StringMessage> message_;
};

#endif