#include "Strategy.h"

bool StrategyBase::addEvent(const BookUpdateEventL10& event) {

    return addEventTemp(event); 
}


bool StrategyBase::addEvent(const TradeEvent& event) {

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