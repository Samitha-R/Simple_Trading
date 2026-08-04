#include "FixMessage.h"
#include "MessageBuilder.h"

void FixMessageHeader::reset()
{
    msgSeqNum_ = 0;
    senderCompID_[0] = '\0';
    targetCompID_[0] = '\0';
    timestamp_ = 0;
}

void FixLogonMessage::reset()
{
    resetSeqNumFlag_ = ResetSeqNumFlag::Types::UNDEFINED;
    heartBtSecond_ = 0;
    nextExpectedSeqNum_ = 0;
    encryptMethod_ = 0;
}

FixMarketUpdate::FixMarketUpdate(SymbolID id, UpdateAction::Types updateAction, EntryType::Types entryType, TimeStamp time, Price price, Volume volume, int position):
    id_(id), updateAction_(updateAction), entryType_(entryType), time_(time), price_(price), volume_(volume), position_(position) {}


FixMarketDataMessage::FixMarketDataMessage(std::size_t numSymbols, const std::vector<SymbolID> &interestedSymbols):
    FixMsgType(FixVersion::FIX44, FixMessageType::MARKET_DATA), fixMarketData_(numSymbols, SymbolMarketData(0))
{
        for (auto symbol : interestedSymbols) {
            fixMarketData_[symbol].reserve(64);
        }
}

void FixMarketDataMessage::reset()
{
    for (auto &symboldata : fixMarketData_) {
        symboldata.clear();
    }
    reqID_ = 0;
}

std::ostream& operator<<(std::ostream& os, const FixMessageHeader& message)
{
    os << "MessageType:" << static_cast<int>(message.getMessageType()) << 
    " SenderCompId:" << message.getSenderCompID() <<
    " TargetCmpId:" << message.getTargetCompID() <<
    " MsgSeqNo:" << message.getMessageSeqNum() <<
    " Timestamp:" << message.getTimeStamp();
    return os;
}

std::ostream& operator<<(std::ostream& os, const FixMarketDataMessage& message)
{
    for (auto &symbolMarketData : message.fixMarketData_) {
        if (!symbolMarketData.empty()) {
            os << symbolMarketData << std::endl;
        }
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const SymbolMarketData& symbolMarketData)
{
    for (auto &marketData : symbolMarketData) {
        os << marketData << std::endl;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const FixMarketUpdate& marketUpdate)
{
    os << "SymbolId:" << static_cast<int>(marketUpdate.getSymbolID()) <<
    " UpdateAction:" << static_cast<int>(marketUpdate.getUpdateAction()) <<
    " EntryType: " << static_cast<char>(marketUpdate.getEntryType()) <<
    " Price: " << marketUpdate.getPrice() <<
    " Volume: " << marketUpdate.getVolume() <<
    " Position: " << marketUpdate.getPosition();

    return os;
}

std::ostream& operator<<(std::ostream& os, const FixLogonMessage& message)
{
    os << " ResetSeqNumFlag:" << static_cast<int>(message.getResetSeqNumFlag()) <<
    " HeartBeatInterval:" << message.getHeartBeatInterval() <<
    " NextExpectedSeqNum:" << message.getNextExpectedSeqNum();
    return os;
}

std::ostream& operator<<(std::ostream& os, const FixHeartBeatMessage& message)
{
    os << " TestId: " << message.getTestID();
    return os;
}

std::ostream& operator<<(std::ostream& os, const FixSnapshotMessage& message)
{
    os << "ReqID:" << message.getReqID() << std::endl;

    for (auto &entry : message.snapShotEntries_) {
        os << entry << std::endl;
    }
    return os;
}

void FixMarketDataRequest::reset()
{
    reqID_ = 0;
    symbols_.clear();
    entryTypes_.clear();
    subscriptionType_ = SubscriptionRequestType::Types::SNAPSHOT_AND_UPDATE;
    marketDepth_ = 0;
    updateType_ = UpdateType::Types::INCREMENTAL_REFRESH;
}


void FixSnapshotMessage::reset()
{
    snapShotEntries_.clear();        
}