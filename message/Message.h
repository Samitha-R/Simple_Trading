#ifndef MESSAGE_H
#define MESSAGE_H

#include <vector>
#include <cstring>
#include <ostream>
#include "FixTags.h"
#include "TypeDef.h"
                            
class OutMessage;
class MessageBuilder;

class MsgBase
{
public:
    MsgBase() = default;
    MsgBase(MessageType type) : type_(type) {}
    MsgBase(MessageType type, MessageProtocol protocol) : type_(type), protocol_(protocol) {}
public:
    MessageType getMessageType() const { return type_; }
    MessageProtocol getProtocol() const { return protocol_; }
    void setProtocol(MessageProtocol protocol) { protocol_ = protocol; }
protected:
    MessageType type_ = MessageType::UNDEFINED;
    MessageProtocol protocol_ = MessageProtocol::UNKNOWN;
};

class MessageHeader : public MsgBase
{
public:
    MessageHeader() : MsgBase(MessageType::HEADER) { senderCompID_[0] = '\0'; targetCompID_[0] = '\0';}
    void reset();
public:
    void setTimeStamp(TimeStamp time) { timestamp_ = time; }
    TimeStamp getTimeStamp() const { return timestamp_; }
    void setMessageSeqNum(std::size_t msgSeqNum) {  msgSeqNum_ =  msgSeqNum; }
    std::size_t getMessageSeqNum() const { return msgSeqNum_; }
    const char* getSenderCompID() const { return senderCompID_; }
    char* getSenderCompID()  { return senderCompID_; }
    const char* getTargetCompID()  const { return targetCompID_; }
    char* getTargetCompID()  { return targetCompID_; }
    std::size_t getSenderCompIDArrLength() { return 30; }
    std::size_t getTargetCompIDArrLength() { return 30; }
    void setTotalMsgSize(std::size_t size) { totalMsgSize_ = size; }
    std::size_t getTotalMsgSize() const { return totalMsgSize_;}
    bool convertToOutMessageFormart(OutMessage& outMessage);
    void setBodyType(MessageType type) { bodyType_ = type; }
    MessageType getBodyType() const { return bodyType_; }
private:
    std::size_t msgSeqNum_ = 0;
    char senderCompID_[30];
    char targetCompID_[30];
    TimeStamp timestamp_ = 0;
    std::size_t totalMsgSize_ = 0;
    MessageType bodyType_ = MessageType::UNDEFINED;
};

class MarketUpdate
{
public:
    MarketUpdate(SymbolID id, UpdateAction updateAction, EntryType entryType, TimeStamp time, Price price, Volume volume, int position);
    MarketUpdate() = default;
    void setSymbolID(SymbolID id) { id_ = id; }
    SymbolID getSymbolID() const { return id_; }
    void setUpdateSeqNum(std::size_t updateSeq) { updateSeqNum_ = updateSeq;}
    std::size_t getUpdateSeqNum() const { return updateSeqNum_;}
    void setUpdateAction(UpdateAction action)  { updateAction_ = action; }
    UpdateAction getUpdateAction() const { return updateAction_; }
    void setEntryType(EntryType entryType) { entryType_ = entryType;}
    EntryType getEntryType() const { return entryType_; }
    void setTimestamp(TimeStamp time) { time_ = time; }
    TimeStamp getTimeStamp() const { return time_; }
    void setPrice(Price price) { price_ = price; }
    Price getPrice() const { return price_; }
    void setVolume(Volume volume) {volume_ = volume; }
    Volume getVolume() const { return volume_; }
    void setPosition(int position) { position_ = position; }
    int getPosition() const { return position_;}
    void setRptSeq(std::size_t rptSeq) { rptSeq_ = rptSeq; }
    RptSeqType getRptSeq() const { return rptSeq_; }
    
private:
    SymbolID id_ = NoSymbolID;
    std::size_t updateSeqNum_ = 0;
    UpdateAction updateAction_ = UpdateAction::UNKNOWN;
    EntryType entryType_ = EntryType::UNKNOWN;
    TimeStamp time_ = 0;
    Price price_ = 0;
    Volume volume_ = 0;
    int position_  = -1;
    RptSeqType rptSeq_ = 0;
};

using SymbolMarketData = std::vector<MarketUpdate>;

class MarketDataMessage : public MsgBase
{
    friend std::ostream& operator<<(std::ostream& os, const MarketDataMessage& marketData);
public:
    MarketDataMessage(std::size_t numSymbols, const std::vector<SymbolID> &interestedSymbols);
    const SymbolMarketData& getFixMarketData(SymbolID id) const { return fixMarketData_[id]; }
    SymbolMarketData& getFixMarketData(SymbolID id) { return fixMarketData_[id]; }
    void addMarketData(const MarketUpdate& data) { fixMarketData_[data.getSymbolID()].push_back(data); }
    void reset();
    void setReqID(std::size_t reqID) { reqID_ = reqID; }
    std::size_t getReqID() const { return reqID_; }
private:
    std::size_t reqID_ = 0;
    std::vector<SymbolMarketData> fixMarketData_;
    
};

class LogonMessage : public MsgBase
{
public:
    void reset();
    LogonMessage() : MsgBase(MessageType::LOGON) {}
    void setResetSeqNumFlag(ResetSeqNumFlag flag) { resetSeqNumFlag_ = flag; }
    ResetSeqNumFlag getResetSeqNumFlag() const { return resetSeqNumFlag_; }
    void setHeartBeatInterval(TimeStamp interval) { heartBtSecond_ = interval; }
    TimeStamp getHeartBeatInterval() const { return heartBtSecond_; }
    void setNextExpectedSeqNum(int seqNum) { nextExpectedSeqNum_ = seqNum; }
    int getNextExpectedSeqNum() const { return nextExpectedSeqNum_; }
    void setEncryptMethod(int encryptMethod) {encryptMethod_ = encryptMethod; }
    int getEncryptMethod() const { return encryptMethod_; }
    void setUserName(std::string_view userName) { userName_ = userName; }
    void setPassWord(std::string_view passWord) { passWord_ = passWord; }
    std::string_view getUserName() const { return userName_; }
    std::string_view getPassWord() const { return passWord_; }
private:
    ResetSeqNumFlag resetSeqNumFlag_ = ResetSeqNumFlag::UNKNOWN;
    TimeStamp heartBtSecond_ = 0;
    int nextExpectedSeqNum_ = 0;
    int encryptMethod_ = 0;
    std::string_view userName_;
    std::string_view passWord_;
};

class LogoutMessage : public MsgBase
{
public:
    LogoutMessage() { reason_[0] = '\0';}
public:
    void reset() { reason_[0] = '\0';}
    char* getReason() { return reason_; }
    const char* getReason() const { return reason_; }
    std::size_t getReasonArrLength() const { return 50; }
private:
    char reason_[50];
};

class HeartBeatMessage : public MsgBase
{
public:
    void reset() { testID_[0] = '\0'; }
    HeartBeatMessage() : MsgBase(MessageType::HEART_BEAT) { testID_[0] = '\0';}
    char* getTestID() { return testID_; }
    const char* getTestID() const { return testID_; }
    std::size_t getTestIdArrLength() { return 30; }
    std::size_t getTestIdLength() const { return strlen(testID_); }
private:
    char testID_[30];
};

class MarketDataRequest : public MsgBase
{
public:
    MarketDataRequest(std::size_t numSymbols = 5, std::size_t numEntryTypes = 3) : MsgBase(MessageType::MARKET_DATA_REUEST) {
        symbols_.reserve(numSymbols);
        entryTypes_.reserve(numEntryTypes);
    }
    void reset();
    void addSymbol(SymbolID id) { symbols_.push_back(id); }
    void addEntryType(EntryType type) { entryTypes_.push_back(type); }
    void setRequestID(std::size_t id) { reqID_ = id; }
    void setSubscriptionType(SubscriptionRequestType type) { subscriptionType_ = type; }
    void setMarketDepth(std::size_t depth) { marketDepth_ = depth; }
    void setUpdateType(UpdateType type) { updateType_ = type; }
    std::size_t getReqID() const { return reqID_; }
    SubscriptionRequestType getSubscriptionType() const { return subscriptionType_; }
    std::size_t getMarketDepth() const { return marketDepth_; }
    UpdateType getUpdateType() const { return updateType_; }
    const std::vector<SymbolID>& getSymbols() const { return symbols_; }
    const std::vector<EntryType>& getEntryTypes() const { return entryTypes_; }
private:
    std::vector<SymbolID> symbols_;
    std::size_t reqID_ = 0;
    SubscriptionRequestType subscriptionType_ = SubscriptionRequestType::SNAPSHOT_AND_UPDATE;
    std::size_t marketDepth_ = 0;
    UpdateType updateType_ = UpdateType::INCREMENTAL_REFRESH;
    std::vector<EntryType> entryTypes_;
};

class SnapshotMessage : public MsgBase
{
friend std::ostream& operator<<(std::ostream& os, const SnapshotMessage& message);
public:
    SnapshotMessage(std::size_t numEntries = 500) : MsgBase(MessageType::SNAPSHOT) { snapShotEntries_.reserve(500); }
    void reset();
    void addSnapshotEntry(const MarketUpdate& entry) { snapShotEntries_.push_back(entry); }
    const std::vector<MarketUpdate>& getSnapshotEntries() const { return snapShotEntries_; }
    void setSymbolID(SymbolID id) { id_ = id; } 
    void setReqID(std::size_t reqID) { reqID_ = reqID; }
    SymbolID getSymbolID() const { return id_; }
    std::size_t getReqID() const { return reqID_; }
private:
    SymbolID id_ = NoSymbolID;
    std::size_t reqID_ = 0;
    std::vector<MarketUpdate> snapShotEntries_;
};

std::ostream& operator<<(std::ostream& os, const MessageHeader& message);
std::ostream& operator<<(std::ostream& os, const MarketDataMessage& message);
std::ostream& operator<<(std::ostream& os, const SymbolMarketData& symbolMarketData);
std::ostream& operator<<(std::ostream& os, const MarketUpdate& marketUpdate);
std::ostream& operator<<(std::ostream& os, const LogonMessage& message);
std::ostream& operator<<(std::ostream& os, const HeartBeatMessage& message);
std::ostream& operator<<(std::ostream& os, const SnapshotMessage& message);
#endif
