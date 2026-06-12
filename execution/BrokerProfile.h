#ifndef BROKER_PROFILE_H
#define BROKER_PROFILE_H

#include <vector>
#include <chrono>
#include "Orders.h"
#include "FastRingBuffer.h"
#include "ExecutionEvents.h"
#include "ExecutionEventNotifier.h"
#include "OrderManager.h"

class BrokerProfile
{
public:
    BrokerProfile(ExecutionEventNotifier& notifier, std::size_t orderEntrySlots, std::size_t maxOrders, std::vector<SymbolState> &symbolStates_);
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
private:
    bool isOrderRateWithinLimit();
    void handleSingleOrder(const SingleOrderEvent& order);
    void handleSingleOrderAck(const SingleOrderAckEvent& event);
    void handleSingleOrderReject(const SingleOrderRejectEvent& event);
    void handleSingleOrderCancel(const CancelOrderEvent& event);
    std::size_t addSingleOrderEdit(const EditOrderEvent& event);
    bool onFilledVolume(const OrderFillEvent& event);
    void onCancelOrderReject(const CancelOrderRejectEvent& event);
    void onCancelOrderAck(const CancelOrderAckEvent& event);
    void onEditOrderReject(const EditOrderRejectEvent& event);
    void onEditOrderAck(const EditOrderAckEvent& event);
    Price calculateFee(const SingleOrderEvent& order);
private:
    Price currentBalance_ = 0;  
    Volume maxOrdersPerSec_ = 1000;
    Volume maxOrderSize_ = 10000;
    Volume minOrderSize_ = 1;
    double maxPriceDeviation_ = 0.05; // 5% price deviation
    ExecutionEventNotifier& eventNotifier_;
    OrderManager orderManager_;
    FastRingBuffer<std::chrono::steady_clock::time_point> orderTimestamps_;
};

#endif