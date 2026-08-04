#include "BrokerProfile.h"


BrokerProfile::BrokerProfile(ExecutionEventNotifier& notifier, std::size_t orderEntrySlots, std::size_t maxOrders, std::vector<SymbolState> &symbolStates_) : 
    eventNotifier_(notifier), orderManager_(orderEntrySlots, maxOrders, symbolStates_, notifier), orderTimestamps_(maxOrdersPerSec_) {}

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

void BrokerProfile::handleSingleOrder(const SingleOrderEvent& order)
{
    if (!isOrderRateWithinLimit()) {
        // Reject order due to rate limit
        auto type = order.getType() == SingleOrderEvent::Type::BUY ? SingleOrderFailureEvent::Type::BUY : SingleOrderFailureEvent::Type::SELL;
        SingleOrderFailureEvent failureEvent(order.getBrokerId(), order.getSymbolId(), type, order.getPrice(), order.getVolume(),
                                             0, SingleOrderFailureEvent::FailureReason::ORDER_RATE_LIMIT_EXCEEDED);
        eventNotifier_.notifySingleOrderFailure(failureEvent);
        return;
    }

    auto sharePrice = order.getPrice() * order.getVolume();
    auto brokerFee = calculateFee(order);
    auto totalCost = sharePrice + brokerFee;

    if (!debitBalance(totalCost)) {
        // Reject order due to insufficient funds
        auto type = order.getType() == SingleOrderEvent::Type::BUY ? SingleOrderFailureEvent::Type::BUY : SingleOrderFailureEvent::Type::SELL;
        SingleOrderFailureEvent failureEvent(order.getBrokerId(), order.getSymbolId(), type, order.getPrice(), order.getVolume(),
                                             0, SingleOrderFailureEvent::FailureReason::INSUFFICIENT_FUNDS);
        eventNotifier_.notifySingleOrderFailure(failureEvent);
        return;
    }

    if (!orderManager_.addSingleOrder(order)) {
        // If order addition fails, credit back the debited amount
        creditBalance(totalCost);
        return;
    }

    
    // Process the order (this is a placeholder, actual processing logic would go here)
    // ...

    // Add the current timestamp to the ring buffer
    orderTimestamps_.push(std::chrono::steady_clock::now());
}

Price BrokerProfile::calculateFee(const SingleOrderEvent& order)
{
    // Placeholder fee calculation logic
    // In a real implementation, this would likely be more complex and based on various factors
    return order.getPrice() * order.getVolume() * 0.001; // Example: 0.1% fee
}



