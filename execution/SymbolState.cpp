#include "SymbolState.h"

void SymbolState::addSingleOrder(BrokerID brokerId, BaseOrder::Type type, Price price, Volume volume)
{
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    pendingShares_[static_cast<std::size_t>(type)] += volume;
    pendingShareValues_[static_cast<std::size_t>(type)] += volume * price;
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
}

void SymbolState::onOrderFill(BrokerID brokerId, BaseOrder::Type type, Price price, Volume filledVolume)
{
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    pendingShares_[static_cast<std::size_t>(type)] -= filledVolume;
    pendingShareValues_[static_cast<std::size_t>(type)] -= filledVolume * price;

    if (type == BaseOrder::Type::BUY) {
        avgBuyPrice_ = ((avgBuyPrice_ * ownShares_) + (filledVolume * price)) / (ownShares_ + filledVolume);
        ownShares_ += filledVolume;
    } else {
        ownShares_ -= filledVolume;
    }
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
}

void SymbolState::onSingleOrderReject(BrokerID brokerId, BaseOrder::Type type, Price price, Volume volume)
{
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    pendingShares_[static_cast<std::size_t>(type)] -= volume;
    pendingShareValues_[static_cast<std::size_t>(type)] -= volume * price;
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
}


void SymbolState::addSingleOrderCancel(BrokerID brokerId, BaseOrder::Type type, Price price, Volume volume)
{
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    pendingCancellations_[static_cast<std::size_t>(type)] += volume;
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
}

void SymbolState::onCancelOrderReject(BrokerID brokerId, BaseOrder::Type type, Price price, Volume volume)
{
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    pendingCancellations_[static_cast<std::size_t>(type)] -= volume;
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
}

void SymbolState::onCancelOrderAck(BrokerID brokerId, BaseOrder::Type type, Price price, Volume volume)
{
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    pendingCancellations_[static_cast<std::size_t>(type)] -= volume;
    pendingShares_[static_cast<std::size_t>(type)] -= volume;
    pendingShareValues_[static_cast<std::size_t>(type)] -= volume * price;
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
}

void SymbolState::addSingleOrderEdit(BrokerID brokerId, BaseOrder::Type type, Price oldPrice, Price newPrice, Volume oldVolume, Volume newVolume)
{
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    pendingShares_[static_cast<std::size_t>(type)] -= oldVolume;
    pendingShares_[static_cast<std::size_t>(type)] += newVolume;
    pendingShareValues_[static_cast<std::size_t>(type)] -= oldVolume * oldPrice;
    pendingShareValues_[static_cast<std::size_t>(type)] += newVolume * newPrice;
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
}


void SymbolState::onEditOrderReject(BrokerID brokerId, BaseOrder::Type type, Price oldPrice, Price newPrice, Volume oldVolume, Volume newVolume)
{
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    pendingShares_[static_cast<std::size_t>(type)] += oldVolume;
    pendingShares_[static_cast<std::size_t>(type)] -= newVolume;
    pendingShareValues_[static_cast<std::size_t>(type)] += oldVolume * oldPrice;
    pendingShareValues_[static_cast<std::size_t>(type)] -= newVolume * newPrice;
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
} 