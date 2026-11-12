#include "Strategy.h"


bool StrategyBase::addEvent(const MarketChangeEvent& event) {

    return addEventTemp(event); 
}

bool StrategyBase::addEvent(const BrokerStatusEvent& event) {

    return addEventTemp(event); 
}

bool StrategyBase::addEvent(const SymbolStatusEvent& event) {

    return addEventTemp(event); 
}

bool StrategyBase::addEvent(const OrderStatusEvent& event) {

    return addEventTemp(event); 
}

bool StrategyBase::addEvent(const OrderReplaceFailureEvent& event) {

    return addEventTemp(event); 
}

template<typename T> void StrategyBase::notifyOrderEventListners(const T& event) const
{
    for (auto& subscriber : orderEventListeners_) {
        subscriber.notify(event);
    }
}