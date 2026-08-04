#ifndef MESSAGE_PARSER_H
#define MESSAGE_PARSER_H

#include <vector>
#include <cstring>
#include "FastRingBuffer.h"
#include "FixMessage.h"
#include "FixTagValueReader.h"

class TagValueReader;

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
    void setMessage(const FixMsgType *message) { message_ = message; }
    const FixMsgType&  getMessage() const { return *message_; }
private:
    const FixMsgType* message_ = nullptr;

};

class MessageParser
{
public:
    MessageParser(const TradeSymbols& symbols, const std::vector<SymbolID>& interestedSymbols);
public:
    const ParseStatus& parseBody();
    const ParseStatus& parseHeader(const char* buffer, std::size_t start, std::size_t end, std::size_t mask);
private:
    const ParseStatus& parseMarketData(TagValueReader &reader);
    const ParseStatus& parseUpdate(TagValueReader& reader, FixMarketUpdate& marketUpdate);
    const ParseStatus& parseHeaderTags(TagValueReader& reader, FixMessageHeader& message);
    const ParseStatus& parseLoginSuccess(TagValueReader &reader);
    const ParseStatus& parseHeartBeat(TagValueReader &reader);
    const ParseStatus& parseLogout(TagValueReader &reader);
    const ParseStatus& parseSnapshot(TagValueReader &reader);
    const ParseStatus& parseSnapshotEntry(TagValueReader &reader, FixMarketUpdate& entry);
    const TagReadError& tagReadError(int tag) { tagReadError_.setTag(tag); return tagReadError_; }
    const TagValueReadError& tagValueReadError(int tag) { tagValueReadError_.setTag(tag); return tagValueReadError_; }
    const MesssageNotComplete& msgNotComplete() { return msgNotComplete_; }
    const IncorrectTagValue& incorrectTagValue(int tag) {incorrectTagValue_.setTag(tag); return incorrectTagValue_;}
    const ParseSuccess& success(const FixMsgType* msg) { success_.setMessage(msg); return success_; }
private:
    FixVersion version_ = FixVersion::FIX44;
    const TradeSymbols &symbols_;
    TagValueReader reader_;
    FixMessageHeader headerMessage_;
    FixMarketDataMessage parsedMarketData_;
    FixLogonMessage loginSuccessMessage_;
    FixLogoutMessage logoutMessage_;
    FixHeartBeatMessage heartBeatMessage_;
    FixSnapshotMessage snapshotMessage_;
    TagReadError tagReadError_;
    TagValueReadError tagValueReadError_;
    MesssageNotComplete msgNotComplete_;
    IncorrectTagValue incorrectTagValue_;
    ParseSuccess success_;
};

#endif
