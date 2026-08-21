#ifndef MARKET_STATE_H
#define MARKET_STATE_H

#include <list>
#include "TypeDef.h"
#include "Liquidity.h"
#include "L2Book.h"
#include "FixMessage.h"
#include "Events.h"

/*class L1Book
{
public:
    L1Book() = default;

    void setBestBidPrice(Price price) { bestBidPrice_ = price; }
    void setBestAskPrice(Price price) { bestAskPrice_ = price; }
    void setBestBidVolume(Volume volume) { bestBidVolume_ = volume; }
    void setBestAskVolume(Volume volume) { bestAskVolume_ = volume; }
    void setLastTrade(const Trade &trade) {lastTrade_ = trade; }
    void setUpdatedTime(TimeStamp time) { time_ = time; }

    Price getBestBidPrice() const { return bestBidPrice_; }
    Price getBestAskPrice() const { return bestAskPrice_; }
    const Trade& getLastTrade() const { return lastTrade_; }
    Volume getBestBidVolume() const { return bestBidVolume_; }
    Volume getBestAskVolume() const { return bestAskVolume_; }
    TimeStamp getTime() const { return time_; }

private:
    Price bestBidPrice_ = 0;
    Price bestAskPrice_ = 0;
    Volume bestBidVolume_ = 0;
    Volume bestAskVolume_ = 0;
    Trade lastTrade_;
    TimeStamp time_ = 0;
};*/

class SymbolMarketStateConfig
{
public:
    SymbolMarketStateConfig(std::size_t tradeSize = 512, std::size_t l2BookSize = 2048, double l2BookTickSize = 0.01) : tradeSize_(tradeSize), l2BookSize_(l2BookSize), l2BookTickSize_(l2BookTickSize) {}
    void setTradeSize(std::size_t size) { tradeSize_ = size; }
    void setL2BookSize(std::size_t size) { l2BookSize_ = size; }
    void setL2BookTickSize(double tickSize) { l2BookTickSize_ = tickSize; }
    std::size_t getTradeSize() const { return tradeSize_; }
    std::size_t getL2BookSize() const { return l2BookSize_; }
    double getL2BookTickSize() const { return l2BookTickSize_; }
private:
    std::size_t tradeSize_ = 512;
    std::size_t l2BookSize_ = 2048;
    double l2BookTickSize_ = 0.01;
};

class SymbolMarketState
{
public:
    SymbolMarketState(const SymbolMarketStateConfig& config);
private:
    enum class Status {INVALID, SNASHOT_ADDED, VALID};
public:
    void clear();
    const L2Book& getL2Book() const { return l2Book_; }
  //  const L1Book& getL1Book() const { return l1Book_; }
    const std::vector<FixMarketUpdate> getTrades() const { return trades_; }
    const FixMarketUpdate& getTrade(std::size_t seqNo) const { return trades_[seqNo % tradeSize_]; }
    std::size_t getTradeSeuence() const { return tradeSeqNo_.load(std::memory_order_acquire); }
    std::size_t getUpdateCount() const { return updateCount_.load(std::memory_order_acquire);}
    LiquidityUpdateStatus addSnapshot(const std::vector<FixMarketUpdate> &marketdata);
    LiquidityUpdateStatus addUpdate(const std::vector<FixMarketUpdate> &marketdata);
private:
    LiquidityUpdateStatus updateBook(const FixMarketUpdate &data);
    void updateTrade(const FixMarketUpdate &data);
private:
    std::size_t lastRptSeq_ = 0;
    std::size_t lastTradeRptSeq_ = 0;
    std::size_t tradeSize_;
    std::atomic<std::size_t> tradeSeqNo_;
    std::atomic<std::size_t> updateCount_;
    std::vector<FixMarketUpdate> trades_;
    std::vector<FixMarketUpdate> tmpBuffer_;
    L2Book l2Book_;
   // L1Book l1Book_;
    bool initialized_ = false;
    Status currentStatus_ = Status::INVALID;
};


#endif