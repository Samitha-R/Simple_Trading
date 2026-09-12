#include "SymbolState.h"

void SymbolState::onSingleOrder(BrokerID brokerId, OrderSide side, Price price, Volume volume)
{
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    pendingShares_[static_cast<std::size_t>(side)] += volume;
    pendingShareValues_[static_cast<std::size_t>(side)] += volume * price;
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
}

void SymbolState::onOrderFill(BrokerID brokerId, OrderSide side, Price price, Volume filledVolume)
{
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    pendingShares_[static_cast<std::size_t>(side)] -= filledVolume;
    pendingShareValues_[static_cast<std::size_t>(side)] -= filledVolume * price;

    if (side == OrderSide::BUY) {
        avgBuyPrice_ = ((avgBuyPrice_ * ownShares_) + (filledVolume * price)) / (ownShares_ + filledVolume);
        ownShares_ += filledVolume;
    } else {
        ownShares_ -= filledVolume;
    }
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
}

void SymbolState::onSingleOrderReject(BrokerID brokerId, OrderSide side, Price price, Volume volume)
{
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    pendingShares_[static_cast<std::size_t>(side)] -= volume;
    pendingShareValues_[static_cast<std::size_t>(side)] -= volume * price;
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
}


void SymbolState::onSingleOrderCancel(BrokerID brokerId, OrderSide side, Price price, Volume volume)
{
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    pendingCancellations_[static_cast<std::size_t>(side)] += volume;
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
}

void SymbolState::onCancelOrderReject(BrokerID brokerId, OrderSide side, Price price, Volume volume)
{
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    pendingCancellations_[static_cast<std::size_t>(side)] -= volume;
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
}

void SymbolState::onCancelOrderAck(BrokerID brokerId, OrderSide side, Price price, Volume volume)
{
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    pendingCancellations_[static_cast<std::size_t>(side)] -= volume;
    pendingShares_[static_cast<std::size_t>(side)] -= volume;
    pendingShareValues_[static_cast<std::size_t>(side)] -= volume * price;
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
}

void SymbolState::onSingleOrderEdit(BrokerID brokerId, OrderSide side, Price oldPrice, Price newPrice, Volume oldVolume, Volume newVolume)
{
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    pendingShares_[static_cast<std::size_t>(side)] -= oldVolume;
    pendingShares_[static_cast<std::size_t>(side)] += newVolume;
    pendingShareValues_[static_cast<std::size_t>(side)] -= oldVolume * oldPrice;
    pendingShareValues_[static_cast<std::size_t>(side)] += newVolume * newPrice;
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
}


void SymbolState::onEditOrderReject(BrokerID brokerId, OrderSide side, Price oldPrice, Price newPrice, Volume oldVolume, Volume newVolume)
{
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    pendingShares_[static_cast<std::size_t>(side)] += oldVolume;
    pendingShares_[static_cast<std::size_t>(side)] -= newVolume;
    pendingShareValues_[static_cast<std::size_t>(side)] += oldVolume * oldPrice;
    pendingShareValues_[static_cast<std::size_t>(side)] -= newVolume * newPrice;
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
} 