#ifndef SYMBOL_STATE_H
#define SYMBOL_STATE_H

#include <atomic>
#include <vector>
#include  "Orders.h"

class SymbolState
{
public:
    SymbolState() : pendingShares_(2, 0), pendingCancellations_(2, 0), pendingShareValues_(2, 0) {};
    SymbolState(SymbolID id) : symbolId_(id), pendingShares_(2, 0), pendingCancellations_(2, 0), pendingShareValues_(2, 0) {}
    void setSymbolID(SymbolID id) { symbolId_ = id; }
    SymbolID getSymbolID() const { return symbolId_; }
    void setMaxAllowedShares(std::size_t maxShares) { maxAllowedShares_ = maxShares; }
    std::size_t getMaxAllowedShares() const { return maxAllowedShares_; }
    
    void onSingleOrder(BrokerID brokerId, OrderSide side, Price price, Volume volume);
    void onSingleOrderReject(BrokerID brokerId, OrderSide side, Price price, Volume volume);
    void onOrderFill(BrokerID brokerId, OrderSide side, Price price, Volume filledCount);

    void onSingleOrderCancel(BrokerID brokerId, OrderSide side, Price price, Volume volume);
    void onCancelOrderReject(BrokerID brokerId, OrderSide side, Price price, Volume volume);
    void onCancelOrderAck(BrokerID brokerId, OrderSide side, Price price, Volume volume);

    void onSingleOrderEdit(BrokerID brokerId, OrderSide side, Price oldPrice, Price newPrice, Volume oldVolume, Volume newVolume);
    void onEditOrderReject(BrokerID brokerId, OrderSide side, Price oldPrice, Price newPrice, Volume oldVolume, Volume newVolume);

    void setBestBidPrice(Price price) { bestBidPrice_ = price; }
    Price getBestBidPrice() const { return bestBidPrice_; }
    Price getAvgBuyPrice() const { return avgBuyPrice_; }
    Volume getOwnShares() const { return ownShares_; }
    Volume getPendingShares(BaseOrder::Type type) const { return pendingShares_[static_cast<std::size_t>(type)]; }
    Volume getPendingCancellations(BaseOrder::Type type) const { return pendingCancellations_[static_cast<std::size_t>(type)]; }
    Price getPendingShareValues(BaseOrder::Type type) const { return pendingShareValues_[static_cast<std::size_t>(type)]; }
    Price getOwnShareValue() const { return ownShares_ * bestBidPrice_; }
    
    std::size_t getUpdateCount() const { return updateCount_.load(std::memory_order_acquire); } 

private:
    SymbolID symbolId_ = NoSymbolID;
    Volume maxAllowedShares_ = 0;
    Volume ownShares_ = 0;
    Price avgBuyPrice_ = 0;
    Price bestBidPrice_ = 0;
    std::vector<Volume> pendingShares_;
    std::vector<Volume> pendingCancellations_;
    std::vector<Volume> pendingShareValues_;
    std::atomic<std::size_t> updateCount_{0};
};

#endif