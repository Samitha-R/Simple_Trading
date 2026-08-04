#ifndef FEED_HANDLER_EVENTS
#define FEED_HANDLER_EVENTS

#include "Events.h"
#include "MarketState.h"

class MarketChangeEvent : public EventBase
{
public:
    MarketChangeEvent() : EventBase(EventType::MARKET_CHANGE) {}
    MarketChangeEvent(std::size_t brokerId, SymbolID symbolID, const SymbolMarketState* symbolMarketState) :
        EventBase(EventType::MARKET_CHANGE), brokerId_(brokerId), symbolID_(symbolID), symbolMarketState_(symbolMarketState) {}
    SymbolID getSymbolID() const { return symbolID_; }
    const SymbolMarketState& getMarketState() const { return *symbolMarketState_; }
private:
    BrokerID brokerId_ = NoBrokerID;
    SymbolID symbolID_ = NoBrokerID;
    const SymbolMarketState* symbolMarketState_ = nullptr;
};


#endif