#ifndef FIX_MESSAGE_H
#define FIX_MESSAGE_H

#include <vector>
#include <cstring>
#include <ostream>
#include "FixTags.h"
#include "TypeDef.h"

enum class FixMessageType { HEART_BEAT = '0',
                            LOGON = 'A',
                            LOGOUT = '5',
                            MARKET_DATA_REUEST = 'V',
                            MARKET_DATA = 'X',
                            SNAPSHOT = 'W',
                            HEADER,
                            UNDEFINED};
                            
class OutMessage;
class MessageBuilder;

class FixMsgType
{
public:
    FixMsgType() = default;
    FixMsgType(FixVersion version, FixMessageType type) : version_(version), type_(type) {}
public:
    FixVersion getVersion() const { return version_; }
    FixMessageType getMessageType() const { return type_; }
protected:
    FixVersion version_ = FixVersion::UNDEFINED;
    FixMessageType type_ = FixMessageType::UNDEFINED;
};

class FixMessageHeader : public FixMsgType
{
public:
    FixMessageHeader() : FixMsgType(FixVersion::FIX44,FixMessageType::HEADER) { senderCompID_[0] = '\0'; targetCompID_[0] = '\0';}
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
    void setBodyType(FixMessageType type) { bodyType_ = type; }
    FixMessageType getBodyType() const { return bodyType_; }
private:
    std::size_t msgSeqNum_ = 0;
    char senderCompID_[30];
    char targetCompID_[30];
    TimeStamp timestamp_ = 0;
    std::size_t totalMsgSize_ = 0;
    FixMessageType bodyType_ = FixMessageType::UNDEFINED;
};

class FixMarketUpdate
{
public:
    FixMarketUpdate(SymbolID id, UpdateAction::Types updateAction, EntryType::Types entryType, TimeStamp time, Price price, Volume volume, int position);
    FixMarketUpdate() = default;
    void setSymbolID(SymbolID id) { id_ = id; }
    SymbolID getSymbolID() const { return id_; }
    void setUpdateSeqNum(std::size_t updateSeq) { updateSeqNum_ = updateSeq;}
    std::size_t getUpdateSeqNum() const { return updateSeqNum_;}
    void setUpdateAction(UpdateAction::Types action)  { updateAction_ = action; }
    UpdateAction::Types getUpdateAction() const { return updateAction_; }
    void setEntryType(EntryType::Types entryType) { entryType_ = entryType;}
    EntryType::Types getEntryType() const { return entryType_; }
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
    UpdateAction::Types updateAction_ = UpdateAction::Types::UNDEFINED;
    EntryType::Types entryType_ = EntryType::Types::UNDEFINED;
    TimeStamp time_ = 0;
    Price price_ = 0;
    Volume volume_ = 0;
    int position_  = -1;
    RptSeqType rptSeq_ = 0;
};

using SymbolMarketData = std::vector<FixMarketUpdate>;

class FixMarketDataMessage : public FixMsgType
{
    friend std::ostream& operator<<(std::ostream& os, const FixMarketDataMessage& marketData);
public:
    FixMarketDataMessage(std::size_t numSymbols, const std::vector<SymbolID> &interestedSymbols);
    const SymbolMarketData& getFixMarketData(SymbolID id) const { return fixMarketData_[id]; }
    SymbolMarketData& getFixMarketData(SymbolID id) { return fixMarketData_[id]; }
    void addMarketData(const FixMarketUpdate& data) { fixMarketData_[data.getSymbolID()].push_back(data); }
    void reset();
    void setReqID(std::size_t reqID) { reqID_ = reqID; }
    std::size_t getReqID() const { return reqID_; }
private:
    std::size_t reqID_ = 0;
    std::vector<SymbolMarketData> fixMarketData_;
    
};

class FixLogonMessage : public FixMsgType
{
public:
    void reset();
    FixLogonMessage() : FixMsgType(FixVersion::FIX44,FixMessageType::LOGON) {}
    void setResetSeqNumFlag(ResetSeqNumFlag::Types flag) { resetSeqNumFlag_ = flag; }
    ResetSeqNumFlag::Types getResetSeqNumFlag() const { return resetSeqNumFlag_; }
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
    ResetSeqNumFlag::Types resetSeqNumFlag_ = ResetSeqNumFlag::Types::UNDEFINED;
    TimeStamp heartBtSecond_ = 0;
    int nextExpectedSeqNum_ = 0;
    int encryptMethod_ = 0;
    std::string_view userName_;
    std::string_view passWord_;
};

class FixLogoutMessage : public FixMsgType
{
public:
    FixLogoutMessage() { reason_[0] = '\0';}
public:
    void reset() { reason_[0] = '\0';}
    char* getReason() { return reason_; }
    const char* getReason() const { return reason_; }
    std::size_t getReasonArrLength() const { return 50; }
private:
    char reason_[50];
};

class FixHeartBeatMessage : public FixMsgType
{
public:
    void reset() { testID_[0] = '\0'; }
    FixHeartBeatMessage() : FixMsgType(FixVersion::FIX44, FixMessageType::HEART_BEAT) { testID_[0] = '\0';}
    char* getTestID() { return testID_; }
    const char* getTestID() const { return testID_; }
    std::size_t getTestIdArrLength() { return 30; }
    std::size_t getTestIdLength() const { return strlen(testID_); }
private:
    char testID_[30];
};

class FixMarketDataRequest : public FixMsgType
{
public:
    FixMarketDataRequest(std::size_t numSymbols = 5, std::size_t numEntryTypes = 3) : FixMsgType(FixVersion::FIX44, FixMessageType::MARKET_DATA_REUEST) {
        symbols_.reserve(numSymbols);
        entryTypes_.reserve(numEntryTypes);
    }
    void reset();
    void addSymbol(SymbolID id) { symbols_.push_back(id); }
    void addEntryType(EntryType::Types type) { entryTypes_.push_back(type); }
    void setRequestID(std::size_t id) { reqID_ = id; }
    void setSubscriptionType(SubscriptionRequestType::Types type) { subscriptionType_ = type; }
    void setMarketDepth(std::size_t depth) { marketDepth_ = depth; }
    void setUpdateType(UpdateType::Types type) { updateType_ = type; }
    std::size_t getReqID() const { return reqID_; }
    SubscriptionRequestType::Types getSubscriptionType() const { return subscriptionType_; }
    std::size_t getMarketDepth() const { return marketDepth_; }
    UpdateType::Types getUpdateType() const { return updateType_; }
    const std::vector<SymbolID>& getSymbols() const { return symbols_; }
    const std::vector<EntryType::Types>& getEntryTypes() const { return entryTypes_; }
private:
    std::vector<SymbolID> symbols_;
    std::size_t reqID_ = 0;
    SubscriptionRequestType::Types subscriptionType_ = SubscriptionRequestType::Types::SNAPSHOT_AND_UPDATE;
    std::size_t marketDepth_ = 0;
    UpdateType::Types updateType_ = UpdateType::Types::INCREMENTAL_REFRESH;
    std::vector<EntryType::Types> entryTypes_;
};

class FixSnapshotMessage : public FixMsgType
{
friend std::ostream& operator<<(std::ostream& os, const FixSnapshotMessage& message);
public:
    FixSnapshotMessage(std::size_t numEntries = 500) : FixMsgType(FixVersion::FIX44, FixMessageType::SNAPSHOT) { snapShotEntries_.reserve(500); }
    void reset();
    void addSnapshotEntry(const FixMarketUpdate& entry) { snapShotEntries_.push_back(entry); }
    const std::vector<FixMarketUpdate>& getSnapshotEntries() const { return snapShotEntries_; }
    void setSymbolID(SymbolID id) { id_ = id; } 
    void setReqID(std::size_t reqID) { reqID_ = reqID; }
    SymbolID getSymbolID() const { return id_; }
    std::size_t getReqID() const { return reqID_; }
private:
    SymbolID id_ = NoSymbolID;
    std::size_t reqID_ = 0;
    std::vector<FixMarketUpdate> snapShotEntries_;
};

std::ostream& operator<<(std::ostream& os, const FixMessageHeader& message);
std::ostream& operator<<(std::ostream& os, const FixMarketDataMessage& message);
std::ostream& operator<<(std::ostream& os, const SymbolMarketData& symbolMarketData);
std::ostream& operator<<(std::ostream& os, const FixMarketUpdate& marketUpdate);
std::ostream& operator<<(std::ostream& os, const FixLogonMessage& message);
std::ostream& operator<<(std::ostream& os, const FixHeartBeatMessage& message);
std::ostream& operator<<(std::ostream& os, const FixSnapshotMessage& message);
#endif
