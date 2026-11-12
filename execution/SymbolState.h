#ifndef SYMBOL_STATE_H
#define SYMBOL_STATE_H

#include <atomic>
#include <vector>
#include <ostream>
#include <array>
#include "TypeDef.h"

class SymbolState
{
public:
    SymbolState() : pendingShares_{0, 0}, pendingCancellations_{0, 0}, pendingShareValues_{0, 0} {};
    SymbolState(SymbolID id) : symbolId_(id), pendingShares_{0, 0}, pendingCancellations_{0, 0}, pendingShareValues_{0, 0} {}
    SymbolState(const SymbolState& state);
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
    Volume getPendingShares(OrderSide side) const { return pendingShares_[static_cast<std::size_t>(side)]; }
    const std::array<Volume,2>& getPendingShares() const { return pendingShares_; }
    Volume getPendingCancellations(OrderSide side) const { return pendingCancellations_[static_cast<std::size_t>(side)]; }
    const std::array<Volume,2>& getPendingCancellations() const { return pendingCancellations_; }
    Price getPendingShareValues(OrderSide side) const { return pendingShareValues_[static_cast<std::size_t>(side)]; }
    const std::array<Price,2>& getPendingShareValues() const { return pendingShareValues_; }
    Price getOwnShareValue() const { return ownShares_ * bestBidPrice_; }

private:
    SymbolID symbolId_ = NoSymbolID;
    Volume maxAllowedShares_ = 0;
    Volume ownShares_ = 0;
    Price avgBuyPrice_ = 0;
    Price bestBidPrice_ = 0;
    std::array<Volume,2> pendingShares_;
    std::array<Volume,2> pendingCancellations_;
    std::array<Price,2> pendingShareValues_;
};

std::ostream& operator<<(std::ostream &stream, const SymbolState& state);
#endif