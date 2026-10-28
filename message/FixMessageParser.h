#ifndef FIX_MESSAGE_PARSER_H
#define FIX_MESSAGE_PARSER_H

#include <vector>
#include <cstring>
#include "FastRingBuffer.h"
#include "Message.h"
#include "FixTagValueReader.h"
#include "ParseStatus.h"

class TagValueReader;

namespace Fix
{
class FixMessageParser
{
public:
    FixMessageParser(const TradeSymbols& symbols, const std::vector<SymbolID>& interestedSymbols);
public:
    const ParseStatus& parseBody();
    const ParseStatus& parseHeader(const char* buffer, std::size_t start, std::size_t end, std::size_t mask);
private:
    const ParseStatus& parseMarketData(TagValueReader &reader);
    const ParseStatus& parseUpdate(TagValueReader& reader, MarketUpdate& marketUpdate);
    const ParseStatus& parseHeaderTags(TagValueReader& reader, MessageHeader& message);
    const ParseStatus& parseLoginSuccess(TagValueReader &reader);
    const ParseStatus& parseHeartBeat(TagValueReader &reader);
    const ParseStatus& parseLogout(TagValueReader &reader);
    const ParseStatus& parseSnapshot(TagValueReader &reader);
    const ParseStatus& parseSnapshotEntry(TagValueReader &reader, MarketUpdate& entry);
    const ParseStatus& parseExecutionReport(TagValueReader &reader);
    const TagReadError& tagReadError(int tag) { tagReadError_.setTag(tag); return tagReadError_; }
    const TagValueReadError& tagValueReadError(int tag) { tagValueReadError_.setTag(tag); return tagValueReadError_; }
    const MesssageNotComplete& msgNotComplete() { return msgNotComplete_; }
    const IncorrectTagValue& incorrectTagValue(int tag) {incorrectTagValue_.setTag(tag); return incorrectTagValue_;}
    const ParseSuccess& success(const MsgBase* msg) { success_.setMessage(msg); return success_; }
private:
    MessageProtocol protocol_ = MessageProtocol::FIX44;
    const TradeSymbols &symbols_;
    TagValueReader reader_;
    MessageHeader headerMessage_;
    MarketDataMessage parsedMarketData_;
    LogonMessage loginSuccessMessage_;
    LogoutMessage logoutMessage_;
    HeartBeatMessage heartBeatMessage_;
    SnapshotMessage snapshotMessage_;
    ExecutionReportMessage executionReportMessage_;
    TagReadError tagReadError_;
    TagValueReadError tagValueReadError_;
    MesssageNotComplete msgNotComplete_;
    IncorrectTagValue incorrectTagValue_;
    ParseSuccess success_;
};

}

#endif
