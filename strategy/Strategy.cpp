#include "Strategy.h"


bool StrategyBase::addMarketChangeEvent(const MarketChangeEvent& event) {
        auto slot = events_.getWriteSlot();

        if (!slot)
            return false;

        slot->getData() = event;
        events_.setWriteComplete(slot);
        return true; 
}

template<typename T> void StrategyBase::notifyOrderEventListners(const T& event) const
{
    for (auto& subscriber : orderEventListeners_) {
        subscriber.notify(event);
    }
}