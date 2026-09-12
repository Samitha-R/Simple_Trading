#include "BrokerProfile.h"


BrokerProfile::BrokerProfile(std::size_t orderEntrySlots, std::size_t maxOrders, std::vector<SymbolState> &symbolStates_) : orderManager_(orderEntrySlots, maxOrders, symbolStates_, *this), orderTimestamps_(maxOrdersPerSec_) {
    orderManagerEventListeners_.reserve(3);
}

void BrokerProfile::creditBalance(const Price& amount)
{
    currentBalance_ += amount;
}

bool BrokerProfile::debitBalance(const Price& amount)
{
    if (currentBalance_ >= amount) {
        currentBalance_ -= amount;
        return true;
    }
    return false;
}

void BrokerProfile::setBalance(const Price& amount)
{
    currentBalance_ = amount;
}

Price BrokerProfile::getBalance() const
{
    return currentBalance_;
}

void BrokerProfile::setMaxOrdersPerSec(unsigned int maxOrders)
{
    maxOrdersPerSec_ = maxOrders;
}

void BrokerProfile::setMaxOrderSize(unsigned int maxSize)
{
    maxOrderSize_ = maxSize;
}

void BrokerProfile::setMinOrderSize(unsigned int minSize)
{
    minOrderSize_ = minSize;
}

void BrokerProfile::setMaxPriceDeviation(double deviation)
{
    maxPriceDeviation_ = deviation;
}

Volume BrokerProfile::getMaxOrderSize() const
{
    return maxOrderSize_;
}

Volume BrokerProfile::getMinOrderSize() const
{
    return minOrderSize_;
}

bool BrokerProfile::isOrderRateWithinLimit()
{
    auto now = std::chrono::steady_clock::now();
    auto oneSecondAgo = now - std::chrono::seconds(1);

    // Remove timestamps older than 1 second
    while (!orderTimestamps_.empty() && orderTimestamps_.front() < oneSecondAgo) {
        orderTimestamps_.popFront();
    }

    return (orderTimestamps_.count() < maxOrdersPerSec_);
}

void BrokerProfile::handleEvent(const SingleOrderEvent& order)
{
    if (!isOrderRateWithinLimit()) {
        // Reject order due to rate limit
        SingleOrderFailureEvent failureEvent(order.getBrokerId(), order.getSymbolId(), order.getSide(), order.getPrice(), order.getVolume(),
                                             0, SingleOrderFailureEvent::FailureReason::ORDER_RATE_LIMIT_EXCEEDED);
        notifyOrderManagerEventListners(failureEvent);
        return;
    }

    
    // Process the order (this is a placeholder, actual processing logic would go here)
    // ...

    // Add the current timestamp to the ring buffer
    orderTimestamps_.push(std::chrono::steady_clock::now());
}

void BrokerProfile::handleEvent(const SingleOrderAckEvent& event)
{
    orderManager_.handleEvent(event);
}

void BrokerProfile::handleEvent(const SingleOrderRejectEvent& event)
{
    orderManager_.handleEvent(event);
}

void BrokerProfile::handleEvent(const CancelOrderEvent& event)
{

    if (!isOrderRateWithinLimit()) {
        // Reject order due to rate limit
        const CommonOrder* parentCommonOrder = orderManager_.getOrderEntry(event.getOrderId());

        if (!parentCommonOrder || !std::holds_alternative<SingleOrder>(*parentCommonOrder)) {

            CancelOrderFailureEvent failureEvent(event.getBrokerId(), NoSymbolID, OrderSide::UNKNOWN, 0, 0, 0,
                                                 CancelOrderFailureEvent::FailureReason::ORDER_NOT_IN_BOOK);
            notifyOrderManagerEventListners(failureEvent);

            return;
        }

        const SingleOrder& singleOrder = std::get<SingleOrder>(*parentCommonOrder);
        CancelOrderFailureEvent failureEvent(event.getBrokerId(), singleOrder.symbolId_, singleOrder.side_, singleOrder.price_, singleOrder.orderVolume_, singleOrder.filledVolume_,
                                                 CancelOrderFailureEvent::FailureReason::ORDER_RATE_LIMIT_EXCEEDED);
        notifyOrderManagerEventListners(failureEvent);

        return;
    }

    if (!orderManager_.handleEvent(event))
        return;

    // Add the current timestamp to the ring buffer
    orderTimestamps_.push(std::chrono::steady_clock::now());
}

void BrokerProfile::handleEvent(const CancelOrderRejectEvent& event)
{
    orderManager_.handleEvent(event);
}

void BrokerProfile::handleEvent(const CancelOrderAckEvent& event)
{
    orderManager_.handleEvent(event);
}

void BrokerProfile::handleEvent(const EditOrderRejectEvent& event)
{
    orderManager_.handleEvent(event);
}

void BrokerProfile::handleEvent(const EditOrderAckEvent& event)
{
    orderManager_.handleEvent(event);
}

void BrokerProfile::handleEvent(const EditOrderEvent& event)
{
    if (!isOrderRateWithinLimit()) {
        // Reject order due to rate limit
        const CommonOrder* parentCommonOrder = orderManager_.getOrderEntry(event.getOrderId());

        if (!parentCommonOrder || !std::holds_alternative<SingleOrder>(*parentCommonOrder)) {

            CancelOrderFailureEvent failureEvent(event.getBrokerId(), NoSymbolID, OrderSide::UNKNOWN, 0, 0, 0,
                                                 CancelOrderFailureEvent::FailureReason::ORDER_NOT_IN_BOOK);
            notifyOrderManagerEventListners(failureEvent);

            return;
        }

        const SingleOrder& singleOrder = std::get<SingleOrder>(*parentCommonOrder);
        CancelOrderFailureEvent failureEvent(event.getBrokerId(), singleOrder.symbolId_, singleOrder.side_, singleOrder.price_, singleOrder.orderVolume_, singleOrder.filledVolume_,
                                                 CancelOrderFailureEvent::FailureReason::ORDER_RATE_LIMIT_EXCEEDED);
        notifyOrderManagerEventListners(failureEvent);

        return;
    }

    if (!orderManager_.handleEvent(event))
        return;

    // Add the current timestamp to the ring buffer
    orderTimestamps_.push(std::chrono::steady_clock::now());
}

Price BrokerProfile::calculateCommission(Price sharePrice, int numShares, OrderSide side)
{
    return sharePrice * numShares * 0.001; // Example: 0.1% commission
}

Price BrokerProfile::calculateCost(Price sharePrice, int numShares, OrderSide side)
{
    if (side == OrderSide::BUY) {
        return sharePrice * numShares + calculateCommission(sharePrice, numShares, side);
    } else {
        return 0 + calculateCommission(sharePrice, numShares, side);
    }
}

void BrokerProfile::registerForOrderManagerEvent(Subscriber subscriber)
{
    orderManagerEventListeners_.push_back(subscriber);
    orderManager_.registerForOrderManagerEvent(subscriber);
}



