#ifndef BROKER_PROFILE_H
#define BROKER_PROFILE_H

#include <vector>
#include <chrono>
#include "Orders.h"
#include "FastRingBuffer.h"
#include "ExecutionEvents.h"
#include "OrderManager.h"

class BrokerProfile
{
public:
    BrokerProfile(std::size_t orderEntrySlots, std::size_t maxOrders, std::vector<SymbolState> &symbolStates_);
    void creditBalance(const Price& amount);
    bool debitBalance(const Price& amount);
    void setBalance(const Price& amount);
    Price getBalance() const;
    void setMaxOrdersPerSec(unsigned int maxOrders);
    void setMaxOrderSize(unsigned int maxSize);
    void setMinOrderSize(unsigned int minSize);
    void setMaxPriceDeviation(double deviation);
    Volume getMaxOrderSize() const;
    Volume getMinOrderSize() const;
    void registerForOrderManagerEvent(Subscriber subscriber);
    Price calculateCommission(Price sharePrice, int numShares, OrderSide side);
    Price calculateCost(Price sharePrice, int numShares, OrderSide side);
private:
    bool isOrderRateWithinLimit();
    void handleEvent(const SingleOrderEvent& order);
    void handleEvent(const SingleOrderAckEvent& event);
    void handleEvent(const SingleOrderRejectEvent& event);
    void handleEvent(const CancelOrderEvent& event);
    void handleEvent(const EditOrderEvent& event);
    bool handleEvent(const OrderFillEvent& event);
    void handleEvent(const CancelOrderRejectEvent& event);
    void handleEvent(const CancelOrderAckEvent& event);
    void handleEvent(const EditOrderRejectEvent& event);
    void handleEvent(const EditOrderAckEvent& event);
    Price calculateFee(const SingleOrderEvent& order);
    template<typename T> void notifyOrderManagerEventListners(const T& event);
private:
    Price currentBalance_ = 0;  
    Volume maxOrdersPerSec_ = 1000;
    Volume maxOrderSize_ = 10000;
    Volume minOrderSize_ = 1;
    double maxPriceDeviation_ = 0.05; // 5% price deviation
    OrderManager orderManager_;
    std::vector<Subscriber> orderManagerEventListeners_;
    FastRingBuffer<std::chrono::steady_clock::time_point> orderTimestamps_;
};

template<typename T> void BrokerProfile::notifyOrderManagerEventListners(const T& event)
{
    for (auto& subscriber : orderManagerEventListeners_) {
        subscriber.notify(event);
    }
}

#endif