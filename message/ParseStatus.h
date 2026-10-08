#ifndef MESSAGE_PARSER_H
#define MESSAGE_PARSER_H

#include "Message.h"

class ParseStatus
{
public:
    enum class Type {SUCCESS, TAG_READ_ERROR, TAG_VALUE_READ_ERROR, MESSAGE_NOT_COMPLETE, INCORRECT_TAG_VALUE};
protected:
    ParseStatus (Type type) : type_(type) {}
public:
    Type getType() const { return type_; }
private:
    Type type_;
};

class MesssageNotComplete : public ParseStatus
{
public:
    MesssageNotComplete() : ParseStatus(Type::MESSAGE_NOT_COMPLETE) {}
};

class TagReadError : public ParseStatus
{
public:
    TagReadError() : ParseStatus(Type::TAG_READ_ERROR) {}
    void setTag(int tag) { tag_ = tag; }
    int getTag() const { return tag_; }
private:
    int tag_ = -1;
};

class TagValueReadError : public ParseStatus
{
public:
    TagValueReadError() : ParseStatus(Type::TAG_VALUE_READ_ERROR) {}
    void setTag(int tag) { tag_ = tag; }
    int getTag() const { return tag_; }
private:
    int tag_ = -1;
};

class IncorrectTagValue : public ParseStatus
{
public:
    IncorrectTagValue() : ParseStatus(Type::INCORRECT_TAG_VALUE) {}
    void setTag(int tag) { tag_ = tag; }
    int getTag() const { return tag_; }
private:
    int tag_ = -1;
};

class ParseSuccess : public ParseStatus
{
public:
    ParseSuccess() : ParseStatus(Type::SUCCESS) {}
    void setMessage(const MsgBase *message) { message_ = message; }
    const MsgBase&  getMessage() const { return *message_; }
private:
    const MsgBase* message_ = nullptr;
};

#endif