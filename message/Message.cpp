#include "Message.h"

void MessageHeader::reset()
{
    msgSeqNum_ = 0;
    senderCompID_[0] = '\0';
    targetCompID_[0] = '\0';
    timestamp_ = 0;
}

void LogonMessage::reset()
{
    resetSeqNumFlag_ = ResetSeqNumFlag::UNKNOWN;
    heartBtSecond_ = 0;
    nextExpectedSeqNum_ = 0;
    encryptMethod_ = 0;
}

MarketUpdate::MarketUpdate(SymbolID id, UpdateAction updateAction, EntryType entryType, TimeStamp time, Price price, Volume volume, int position):
    id_(id), updateAction_(updateAction), entryType_(entryType), time_(time), price_(price), volume_(volume), position_(position) {}


MarketDataMessage::MarketDataMessage(std::size_t numSymbols, const std::vector<SymbolID> &interestedSymbols):
    MsgBase(MessageType::MARKET_DATA), fixMarketData_(numSymbols, SymbolMarketData(0))
{
        for (auto symbol : interestedSymbols) {
            fixMarketData_[symbol].reserve(64);
        }
}

void MarketDataMessage::reset()
{
    for (auto &symboldata : fixMarketData_) {
        symboldata.clear();
    }
    reqID_ = 0;
}

void MarketDataRequest::reset()
{
    reqID_ = 0;
    symbols_.clear();
    entryTypes_.clear();
    subscriptionType_ = SubscriptionRequestType::SNAPSHOT_AND_UPDATE;
    marketDepth_ = 0;
    updateType_ = UpdateType::INCREMENTAL_REFRESH;
}


void SnapshotMessage::reset()
{
    snapShotEntries_.clear();        
}

std::ostream& operator<<(std::ostream& os, const MessageHeader& message)
{
    os << "MessageType:" << static_cast<int>(message.getMessageType()) << 
    " SenderCompId:" << message.getSenderCompID() <<
    " TargetCmpId:" << message.getTargetCompID() <<
    " MsgSeqNo:" << message.getMessageSeqNum() <<
    " Timestamp:" << message.getTimeStamp();
    return os;
}

std::ostream& operator<<(std::ostream& os, const MarketDataMessage& message)
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

std::ostream& operator<<(std::ostream& os, const MarketUpdate& marketUpdate)
{
    os << "SymbolId:" << static_cast<int>(marketUpdate.getSymbolID()) <<
    " UpdateAction:" << static_cast<int>(marketUpdate.getUpdateAction()) <<
    " EntryType: " << static_cast<char>(marketUpdate.getEntryType()) <<
    " Price: " << marketUpdate.getPrice() <<
    " Volume: " << marketUpdate.getVolume() <<
    " Position: " << marketUpdate.getPosition();

    return os;
}

std::ostream& operator<<(std::ostream& os, const LogonMessage& message)
{
    os << " ResetSeqNumFlag:" << static_cast<int>(message.getResetSeqNumFlag()) <<
    " HeartBeatInterval:" << message.getHeartBeatInterval() <<
    " NextExpectedSeqNum:" << message.getNextExpectedSeqNum();
    return os;
}

std::ostream& operator<<(std::ostream& os, const HeartBeatMessage& message)
{
    os << " TestId: " << message.getTestID();
    return os;
}

std::ostream& operator<<(std::ostream& os, const SnapshotMessage& message)
{
    os << "ReqID:" << message.getReqID() << std::endl;

    for (auto &entry : message.snapShotEntries_) {
        os << entry << std::endl;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const NewOrderSingleMessage& message)
{
    os << "ClientOrderID:" << message.getClientOrderID() <<
    " SymbolID:" << message.getSymbolID() <<
    " Side:" << static_cast<int>(message.getSide()) <<
    " OrderTime:" << message.getOrderTime() <<
    " OrderVolume:" << message.getOrderVolume() <<
    " OrderPrice:" << message.getOrderPrice() <<
    " OrderType:" << static_cast<int>(message.getOrderType()) <<
    " TimeInForce:" << static_cast<int>(message.getTimeInForce());
    return os;
}