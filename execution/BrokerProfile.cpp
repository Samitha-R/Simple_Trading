#include "BrokerProfile.h"


BrokerProfile::BrokerProfile(BrokerID id, std::size_t orderEntrySlots, std::size_t maxOrders, std::vector<SymbolState> &symbolStates_, ExecutionGateway& executionGateway) : id_(id), orderManager_(orderEntrySlots, maxOrders, symbolStates_, *this), orderTimestamps_(maxOrdersPerSec_), executionGateway_(executionGateway) {
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
        OrderStatusEvent statusEvent(order.getStrategyID(), order.getStrategyOrderID(), 0, order.getOrderPrice(), order.getOrderVolume(), 0, static_cast<int>(OrderStatus::REJECTED));
        statusEvent.setOrderFailureReason(OrderFailureReason::ORDER_RATE_LIMIT_EXCEEDED);
        notifyOrderManagerEventListners(statusEvent);
        return;
    }

    auto orderID = orderManager_.handleEvent(order);

    if (!orderID)
        return;
    
    NewOrderSingleMessage newOrderMsg(orderID, order.getSymbolID(), order.getSide(), std::chrono::system_clock::now(), order.getOrderVolume(), order.getOrderPrice(), order.getOrderType(), order.getTimeInForce());
    executionGateway_.addMessageToSend(id_, newOrderMsg);
    orderTimestamps_.push(std::chrono::steady_clock::now());
}

void BrokerProfile::handleEvent(const CancelOrderEvent& event)
{
    if (!isOrderRateWithinLimit()) {
        // Reject order due to rate limit
        auto originalOrder = orderManager_.getOrderEntry(event.getOrderId());

        if (!originalOrder) {
            OrderReplaceFailureEvent replaceFailureEvent(event.getStrategyID(), event.getStrategyOrderID(), event.getOrderId(), OrderFailureReason::ORDER_NOT_IN_BOOK);
            notifyOrderManagerEventListners(replaceFailureEvent);

            return;
        }

        OrderReplaceFailureEvent replaceFailureEvent(originalOrder->getStrategyId(), originalOrder->getStrategyOrderId(), originalOrder->getOrderId(), OrderFailureReason::ORDER_RATE_LIMIT_EXCEEDED);
        notifyOrderManagerEventListners(replaceFailureEvent);

        return;
    }

    auto orderID = orderManager_.handleEvent(event);

    if (!orderID)
        return;
 
    auto originalOrder = orderManager_.getOrderEntry(event.getOrderId());
    OrderCancelMessage cancelMessage(orderID, event.getOrderId(), originalOrder->getBrokerOrderId().data(), originalOrder->getSymbolId(), originalOrder->getSide(), originalOrder->getOrderVolume(), std::chrono::system_clock::now());
    executionGateway_.addMessageToSend(id_, cancelMessage);
    orderTimestamps_.push(std::chrono::steady_clock::now());
}

void BrokerProfile::handleEvent(const EditOrderEvent& event)
{
    if (!isOrderRateWithinLimit()) {
        // Reject order due to rate limit
        auto originalOrder = orderManager_.getOrderEntry(event.getOrderId());

        if (!originalOrder) {

            OrderReplaceFailureEvent replaceFailureEvent(event.getStrategyID(), event.getStrategyOrderID(), event.getOrderId(), OrderFailureReason::ORDER_NOT_IN_BOOK);
            notifyOrderManagerEventListners(replaceFailureEvent);

            return;
        }

        OrderReplaceFailureEvent replaceFailureEvent(originalOrder->getStrategyId(), originalOrder->getStrategyOrderId(), originalOrder->getOrderId(), OrderFailureReason::ORDER_RATE_LIMIT_EXCEEDED);
        notifyOrderManagerEventListners(replaceFailureEvent);

        return;
    }

    auto orderID = orderManager_.handleEvent(event);

    if (!orderID)
        return;

    auto originalOrder = orderManager_.getOrderEntry(event.getOrderId());
    
    OrderReplaceMessage replaceMessage(orderID, event.getOrderId(), originalOrder->getBrokerOrderId().data(), originalOrder->getSymbolId(), originalOrder->getSide(),
                                       std::chrono::system_clock::now(), originalOrder->getNewVolume(), originalOrder->getNewPrice(),
                                       originalOrder->getOrderType(), originalOrder->getNewTimeInForce());
    replaceMessage.setStopPrice(originalOrder->getNewStopPrice());
    executionGateway_.addMessageToSend(id_, replaceMessage);
    orderTimestamps_.push(std::chrono::steady_clock::now());
}

void BrokerProfile::handleEvent(const ExecutionReportEvent& event)
{
    orderManager_.handleEvent(event);
}

void BrokerProfile::handleEvent(const OrderCancelRejectEvent& event)
{
     orderManager_.handleEvent(event);
}

Price BrokerProfile::calculateCommission(Price sharePrice, int numShares, OrderSide side)
{
    return sharePrice * numShares * Decimal<long long, 3>(1); // Example: 0.1% commission
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

std::ostream& operator<<(std::ostream &os, const BrokerProfile& bp)
{
    os << "balance=" << bp.currentBalance_  << std::endl;
    os << bp.orderManager_;
    return os;
}


