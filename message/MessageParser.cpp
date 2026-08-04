#include "MessageParser.h"
#include "FixTags.h"
#include <iostream>

using u64 = uint64_t;

// Fast ASCII to int (fixed width)
inline int toInt(const char* p, int len)
{
    int v = 0;
    for (int i = 0; i < len; ++i)
        v = v * 10 + (p[i] - '0');
    return v;
}

inline int64_t daysFromCivil(int y, unsigned m, unsigned d)
{
    y -= m <= 2;
    const int era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);
    const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2)/5 + d - 1;
    const unsigned doe = yoe * 365 + yoe/4 - yoe/100 + doy;
    return era * 146097 + static_cast<int>(doe) - 719468;
}

TimeStamp fixTimestampToNs(const char* ts)
{
    // YYYYMMDD-HH:MM:SS[.fraction]

    int year  = toInt(ts + 0, 4);
    int month = toInt(ts + 4, 2);
    int day   = toInt(ts + 6, 2);

    int hour  = toInt(ts + 9, 2);
    int min   = toInt(ts + 12, 2);
    int sec   = toInt(ts + 15, 2);

    int64_t days = daysFromCivil(year, month, day);

    auto total_ns = static_cast<TimeStamp>(days) * 86400ull;
    total_ns += hour * 3600ull;
    total_ns += min * 60ull;
        total_ns += sec;

    total_ns *= 1'000'000'000ull;

    // Fractional part
    const char* frac = ts + 17;

    if (*frac == '.')
    {
        ++frac;
        u64 frac_ns = 0;
        int digits = 0;

        while (*frac >= '0' && *frac <= '9' && digits < 9)
        {
            frac_ns = frac_ns * 10 + (*frac - '0');
            ++frac;
            ++digits;
        }

        // scale if fewer than 9 digits
        while (digits < 9)
        {
            frac_ns *= 10;
            ++digits;
        }

        total_ns += frac_ns;
    }

    return total_ns;
}

MessageParser::MessageParser(const TradeSymbols& symbols, const std::vector<SymbolID> &interestedSymbols) :
    symbols_(symbols), parsedMarketData_(symbols.getNumSymbols(), interestedSymbols)
{

}

const ParseStatus& MessageParser::parseHeader(const char* buffer, std::size_t start, std::size_t end, std::size_t mask)
{
    reader_ = TagValueReader(buffer, start, end, mask);
    if (reader_.getRemainBytes() < 16)
        return msgNotComplete_;

    
    int tag;

    if (!reader_.getTag(tag) || (tag != BeginString::id_))
        return tagReadError(BeginString::id_);

    char version[20];

    if (!reader_.getValue(version, 20))
         return tagValueReadError(BeginString::id_);
    
    FixVersion versionEnum = toEnum(version);

    if (versionEnum != version_)
        return incorrectTagValue(BeginString::id_);
    
    if (!reader_.getTag(tag) || (tag != BodyLength::id_))
        return tagReadError(BodyLength::id_);

    int bodyLegth;

    if (!reader_.getValue(bodyLegth))
        return tagValueReadError(BodyLength::id_);

    if (bodyLegth + 6 > reader_.getRemainBytes())
        return msgNotComplete(); 

    int msgTypeBeginPos = reader_.getParseBytes();

    reader_.moveReadPosTo( msgTypeBeginPos + bodyLegth);

    auto checksumTagPos =  reader_.getParseBytes();

    if (!reader_.getTag(tag) || tag != CheckSum::id_)
        return tagReadError(CheckSum::id_);
    
    int checksum;

    if (!reader_.getValue(checksum))
        return tagValueReadError(CheckSum::id_);

    auto totalMsgSize = reader_.getParseBytes();

    int checkSumCal = 0;

    for (std::size_t i = 0;  i < checksumTagPos; ++i) {
        unsigned char val = reader_[i];
        checkSumCal += val;
    }

    checkSumCal = checkSumCal % 256;

    if (checkSumCal != checksum) {
        return incorrectTagValue(CheckSum::id_);
    }

    reader_.moveReadPosTo(msgTypeBeginPos);

    headerMessage_.reset();
    headerMessage_.setTotalMsgSize(totalMsgSize);
    return parseHeaderTags(reader_, headerMessage_);
}

const ParseStatus& MessageParser::parseBody()
{
    auto msgTYpe = headerMessage_.getBodyType();


    switch (msgTYpe) {
        case FixMessageType::MARKET_DATA: 
            return parseMarketData(reader_);
        case FixMessageType::LOGON:
            return parseLoginSuccess(reader_);
        case FixMessageType::HEART_BEAT:
            return parseHeartBeat(reader_);
        case FixMessageType::SNAPSHOT:
            return parseSnapshot(reader_);
        default:
            return incorrectTagValue(MsgType::id_);
    }

}

const ParseStatus& MessageParser::parseHeaderTags(TagValueReader& reader_, FixMessageHeader& message)
{
    int tag;

    while(true) {

        if (!reader_.getTag(tag))
            return tagReadError(tag);

        switch (tag) {
            case MsgType::id_: {
                char value;

                if (!reader_.getValue(value))
                    return tagValueReadError(tag);
                message.setBodyType(static_cast<FixMessageType>(value));
                break;
            }
            case SendingTime::id_: {
                char time[40]; 
                int length = reader_.getValue(time, 40);

                if (length > 0) {
                    message.setTimeStamp(fixTimestampToNs(time));
                } else {
                    return tagValueReadError(tag);
                }
                break;
            }
            case SenderCompID::id_: {

                if (!reader_.getValue(message.getSenderCompID(), message.getSenderCompIDArrLength())) {
                    return tagValueReadError(tag);
                }
                break;
            }
            case TargetCompID::id_: {
                
                if (!reader_.getValue(message.getTargetCompID(),  message.getTargetCompIDArrLength())) {
                    return tagValueReadError(tag);
                }
                break;

            }
            case MsgSeqNum::id_: {
                int msgSeqNum;

                if (reader_.getValue(msgSeqNum)) {
                    message.setMessageSeqNum(msgSeqNum);
                } else {
                    return tagValueReadError(tag);
                }
                break;
            }
            case PossDupFlag::id_:
            case PossResend::id_:
            case LastMsgSeqNumProcessed::id_:
            case SenderSubID::id_:
            case TargetSubID::id_:
            case OrigSendingTime::id_:
                reader_.moveToNextTag();
                break;
            default:
                reader_.moveReadPosTo(reader_.getLastTagPos());
                return success(&message);  
        }
    }
}

const ParseStatus& MessageParser::parseMarketData(TagValueReader& reader_)
{
    parsedMarketData_.reset();

    /*auto& status = parseBaseTags(reader_,parsedMarketData_);

    if (status.getType() != ParseStatus::Type::SUCCESS)
        return status;*/

    int tag;
    int numEntries;

    while(true) {

        if (!reader_.getTag(tag))
            return tagValueReadError(tag);

        switch (tag) {
                case ReqID::id_ : {
                int reqID;

                if (!reader_.getValue(reqID))
                    return tagValueReadError(tag);

                parsedMarketData_.setReqID(reqID);
                break;
            }
            case NoEntries::id_:
                if (!reader_.getValue(numEntries)) {
                    return tagValueReadError(tag);
                }
                break;
            case UpdateAction::id_: {
                int updateAction;

                if (!reader_.getValue(updateAction)) {
                    return tagValueReadError(tag);
                }

                FixMarketUpdate update;
                update.setUpdateAction(static_cast<UpdateAction::Types>(updateAction));  
                auto &status = parseUpdate(reader_, update);

                if (status.getType() != ParseStatus::Type::SUCCESS)
                    return status;

                if (update.getTimeStamp() == 0)
                    update.setTimestamp(headerMessage_.getTimeStamp());
                
                update.setUpdateSeqNum(headerMessage_.getMessageSeqNum());
                
                parsedMarketData_.addMarketData(update);
                break;
            }
            default:
                reader_.moveReadPosTo(reader_.getLastTagPos());
                return success(&parsedMarketData_);
        }
    }

}

const ParseStatus& MessageParser::parseUpdate(TagValueReader& reader_, FixMarketUpdate& marketUpdate)
{
    while(true) {
        int tag;

        if (!reader_.getTag(tag))
            return tagReadError(tag);

        switch (tag) {
            case EntryType::id_: {
                char entryType;
                if (!reader_.getValue(entryType)) {
                    return tagValueReadError(tag);
                }
                marketUpdate.setEntryType(static_cast<EntryType::Types>(entryType));
                break;
            }
            case Symbol::id_:{
                char symbol[20]; 
                int length = reader_.getValue(symbol, 20);

                if (!length)
                    return tagValueReadError(tag);

                SymbolID id = symbols_.getSymbolID(symbol);
                marketUpdate.setSymbolID(id);
                break;
            }
            case LastPrice::id_:
            case EntryPrice::id_: {
                double price;
                if (!reader_.getValue(price)) {
                    return tagValueReadError(tag);
                }
                marketUpdate.setPrice(price);
                break;
            }
            case EntryPosition::id_: {
                int position;
                if (!reader_.getValue(position)) {
                    return tagValueReadError(tag);
                }
                marketUpdate.setPosition(position);
                break;
            }
            case RptSeq::id_ : {
                int rptSeq;

                if (!reader_.getValue(rptSeq))
                    return tagValueReadError(tag);
                
                marketUpdate.setRptSeq(rptSeq);
                break;
            } 
            case EntryID::id_:
            case NumberOfOrders::id_:
                reader_.moveToNextTag();
                break;

            case LastQuantity::id_: 
            case EntrySize::id_: {
                int value;
                if (!reader_.getValue(value)) {
                    return tagValueReadError(tag);
                }
                marketUpdate.setVolume(value);
                break;
            }
            case TransactionTime::id_: {
                char value[40];
                int length = reader_.getValue(value, 40);

                if (!length)
                    return tagValueReadError(tag);

                marketUpdate.setTimestamp(fixTimestampToNs(value));
                break;
            }
            default:
                reader_.moveReadPosTo(reader_.getLastTagPos());
                return success(nullptr);

        }
    }
}

const ParseStatus& MessageParser::parseLoginSuccess(TagValueReader &reader_)
{
    loginSuccessMessage_.reset();

    int tag;

    while(true) {

        if (!reader_.getTag(tag))
            return tagValueReadError(tag);

        switch (tag) {
            case ResetSeqNumFlag::id_: {
                char val;

                if (!reader_.getValue(val)) {
                    return tagValueReadError(tag);
                }

                loginSuccessMessage_.setResetSeqNumFlag(static_cast<ResetSeqNumFlag::Types>(val));
                break;
            }
            case HeartBtInt::id_: {
                int val;

                if (!reader_.getValue(val)) {
                    return tagValueReadError(tag);
                }

                loginSuccessMessage_.setHeartBeatInterval(val);
                break;
            }
            case NextExpectedSeqNum::id_: {
                int val;

                if (!reader_.getValue(val)) {
                    return tagValueReadError(tag);
                }

                loginSuccessMessage_.setNextExpectedSeqNum(val);
                break;
            }
            case EncryptMethod::id_: {
                int val;

                if (!reader_.getValue(val)) {
                    return tagValueReadError(tag);
                }

                loginSuccessMessage_.setEncryptMethod(val);
                break;
            }
            default:
                reader_.moveReadPosTo(reader_.getLastTagPos());
                return success(&loginSuccessMessage_);
        }
    }
        
}

const ParseStatus& MessageParser::parseLogout(TagValueReader &reader_)
{
    logoutMessage_.reset();

    int tag;

    while(true) {

        if (!reader_.getTag(tag))
            return tagValueReadError(tag);

        switch (tag) {
            case Text::id_: {

                if (!reader_.getValue(logoutMessage_.getReason(), logoutMessage_.getReasonArrLength())) {
                    return tagValueReadError(tag);
                }
                break;
            }
            default:
                reader_.moveReadPosTo(reader_.getLastTagPos());
                return success(&logoutMessage_);
        }
    }
}

const ParseStatus& MessageParser::parseHeartBeat(TagValueReader &reader_)
{
    heartBeatMessage_.reset();
    
    int tag;

    while(true) {

        if (!reader_.getTag(tag))
            return tagValueReadError(tag);

        switch (tag) {
            case TestReqID::id_: {

                auto length = reader_.getValue(heartBeatMessage_.getTestID(), heartBeatMessage_.getTestIdArrLength());

                if (!length) {
                    return tagValueReadError(tag);
                }
                break;
            }
            default:
                reader_.moveReadPosTo(reader_.getLastTagPos());
                return success(&heartBeatMessage_);
        }
    }
}

const ParseStatus& MessageParser::parseSnapshot(TagValueReader &reader_)
{
    snapshotMessage_.reset();
    int tag;
    int numEntries;
    SymbolID id = NoSymbolID;
    int rptSeq = 0;
    while(true) {

        if (!reader_.getTag(tag))
            return tagValueReadError(tag);

        switch (tag) {
            case Symbol::id_:{
                char symbol[20]; 
                int length = reader_.getValue(symbol, 20);

                if (!length)
                    return tagValueReadError(tag);

                id = symbols_.getSymbolID(symbol);
                snapshotMessage_.setSymbolID(id);
                break;
            }
            case ReqID::id_ : {
                int reqID;

                if (!reader_.getValue(reqID))
                    return tagValueReadError(tag);

                snapshotMessage_.setReqID(reqID);
                break;
            }
            case NoEntries::id_ : {

                if (!reader_.getValue(numEntries)) {
                    return tagValueReadError(tag);
                }

                break;
            }
            case EntryType::id_ : {
                char entryType;

                if (!reader_.getValue(entryType)) {
                    return tagValueReadError(tag);
                }

                FixMarketUpdate snapshotEntry;
                snapshotEntry.setEntryType(static_cast<EntryType::Types>(entryType));
                auto& status = parseSnapshotEntry(reader_, snapshotEntry);

                if (status.getType() != ParseStatus::Type::SUCCESS)
                    return status;

                
                if (snapshotEntry.getTimeStamp() == 0)
                    snapshotEntry.setTimestamp(headerMessage_.getTimeStamp());

                snapshotEntry.setSymbolID(id);
                snapshotMessage_.addSnapshotEntry(snapshotEntry);

                break;
            }
            default:
                reader_.moveReadPosTo(reader_.getLastTagPos());
                return success(&snapshotMessage_);
        }
    }

}

const ParseStatus& MessageParser::parseSnapshotEntry(TagValueReader &reader_, FixMarketUpdate& entry)
{
   while(true) {
        int tag;

        if (!reader_.getTag(tag))
            return tagReadError(tag);

        switch (tag) {
            case LastPrice::id_:
            case EntryPrice::id_: {
                double price;
                if (!reader_.getValue(price)) {
                    return tagValueReadError(tag);
                }
                entry.setPrice(price);
                break;
            }
            case EntryPosition::id_: {
                int position;
                if (!reader_.getValue(position)) {
                    return tagValueReadError(tag);
                }
                entry.setPosition(position);
                break;
            }
            case LastQuantity::id_: 
            case EntrySize::id_: {
                int value;
                if (!reader_.getValue(value)) {
                    return tagValueReadError(tag);
                }
                entry.setVolume(value);
                break;
            }
            case EntryTime::id_: {
                char value[40];
                int length = reader_.getValue(value, 40);

                if (!length)
                    return tagValueReadError(tag);

                entry.setTimestamp(fixTimestampToNs(value));
                break;
            }
            case RptSeq::id_ : {
                int rptSeq;
                if (!reader_.getValue(rptSeq))
                    return tagValueReadError(tag);

                entry.setRptSeq(rptSeq);
                break;
            } 
            default:
                reader_.moveReadPosTo(reader_.getLastTagPos());
                return success(nullptr);
        }
    }
}