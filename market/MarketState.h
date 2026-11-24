#ifndef MARKET_STATE_H
#define MARKET_STATE_H

#include <list>
#include "TypeDef.h"
#include "Liquidity.h"
#include "Message.h"
#include "Events.h"
#include "FeedHandlerEvents.h"

class SymbolMarketStateConfig
{
public:
    SymbolMarketStateConfig(std::size_t tmpBufferSize_ = 128, std::size_t l2BookSize = 2048, int l2BookTickSize = 100) : tmpBufferSize_(tmpBufferSize_), l2BookSize_(l2BookSize), l2BookTickSize_(l2BookTickSize) {}
    void setTradeSize(std::size_t size) { tmpBufferSize_ = size; }
    void setL2BookSize(std::size_t size) { l2BookSize_ = size; }
    void setL2BookTickSize(double tickSize) { l2BookTickSize_ = tickSize; }
    std::size_t getTradeSize() const { return tmpBufferSize_; }
    std::size_t getL2BookSize() const { return l2BookSize_; }
    int getL2BookTickSize() const { return l2BookTickSize_; }
private:
    std::size_t tmpBufferSize_;
    std::size_t l2BookSize_;
    int l2BookTickSize_;
};

class SymbolMarketState
{
public:
    SymbolMarketState(const SymbolMarketStateConfig& config, BrokerID brokerID, SymbolID symbolID);
private:
    enum class Status {INVALID, SNASHOT_ADDED, VALID};
public:
    void clear();
    LiquidityUpdateStatus addSnapshot(const std::vector<MarketUpdate> &marketdata);
    LiquidityUpdateStatus addUpdate(const std::vector<MarketUpdate> &marketdata);
    void registerForMarketEvents(Subscriber subscriber) {  marketUpdateSubscribers_.push_back(subscriber); }
private:
    void notifyMarketChanges(const EventBase& event);
    void sendBookUpdate();
    LiquidityUpdateStatus updateBook(const MarketUpdate &data);
private:
    BrokerID brokerID_;
    SymbolID symbolID_;
    std::size_t lastRptSeq_ = 0;
    std::size_t lastTradeRptSeq_ = 0;
    std::vector<MarketUpdate> tmpBuffer_;
    std::vector<Subscriber> marketUpdateSubscribers_;
    Status currentStatus_ = Status::INVALID;
    Liquidity<LiquidityType::BID> bidLiquidity_;
    Liquidity<LiquidityType::ASK> askLiquidity_;
    TimeStamp lastUpdatedTime_;
};


#endif