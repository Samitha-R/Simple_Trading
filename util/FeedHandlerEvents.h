#ifndef FEED_HANDLER_EVENTS
#define FEED_HANDLER_EVENTS

#include "Events.h"
#include "MarketState.h"

template<unsigned int Level> class BookUpdateEvent : public EventBase
{
public:
    BookUpdateEvent() : EventBase(EventType::BOOK_UPDATE) {}
    BookUpdateEvent(std::size_t brokerId, SymbolID symbolID) :
        EventBase(EventType::BOOK_UPDATE), brokerID_(brokerId), symbolID_(symbolID) {}
    SymbolID getSymbolID() const { return symbolID_; }
    BrokerID getBrokerID() const { return brokerID_; }
    const LiquiditySnapshot<Level,LiquidityType::BID>& getBidLiquidity() const { return bidLiquidity_; }
    LiquiditySnapshot<Level,LiquidityType::BID>& getBidLiquidity() { return bidLiquidity_; }
    const LiquiditySnapshot<Level,LiquidityType::ASK>& getAskLiquidity() const { return askLiquidity_; }
    LiquiditySnapshot<Level,LiquidityType::ASK>& getAskLiquidity() { return askLiquidity_; }
    void setUpdatedTime(TimeStamp updateTime) { updatedTime_ = updateTime; }
    TimeStamp getUpdatedTime() const { return updatedTime_; }
private:
    BrokerID brokerID_ = NoBrokerID;
    SymbolID symbolID_ = NoBrokerID;
    LiquiditySnapshot<Level,LiquidityType::BID> bidLiquidity_;
    LiquiditySnapshot<Level, LiquidityType::ASK> askLiquidity_;
    TimeStamp updatedTime_ = 0;
};

class TradeEvent : public EventBase
{
public:
    TradeEvent() : EventBase(EventType::TRADE) {}
    TradeEvent(BrokerID brokerId, const MarketUpdate& trade) : EventBase(EventType::TRADE), brokerId_(brokerId), trade_(trade) {}
    const MarketUpdate& getTrade() const { return trade_; }
    BrokerID getBrokerID() const { return brokerId_; }
    SymbolID getSymbolID() const { return trade_.getSymbolID(); }
private:
    BrokerID brokerId_ = NoBrokerID;
    MarketUpdate trade_;
};

using BookUpdateEventL10 = BookUpdateEvent<10>;

#endif