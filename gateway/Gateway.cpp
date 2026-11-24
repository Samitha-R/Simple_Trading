#include "Gateway.h"

void MarketGateway::registerForMarketEvents(Subscriber subscriber)
{
    for (auto &broker : brokers_) {
        broker.registerForMarketEvents(subscriber);
    }
}

bool MarketGateway::addMessageToSend(BrokerID id, const MarketDataRequestMessage& msg)
{
    if (id >= brokers_.size())
        return false;

    auto& broker = brokers_[id];

   return  broker.addMessageToSend(msg);
}

void ExecutionGateway::registerForExecutionEvents(Subscriber subscriber)
{
    for (auto &broker : brokers_) {
        broker.registerForExecutionEvents(subscriber);
    }
}

bool ExecutionGateway::addMessageToSend(BrokerID id, const NewOrderSingleMessage& msg)
{
    if (id >= brokers_.size())
        return false;

    auto& broker = brokers_[id];

    return broker.addMessageToSend(msg);
}

bool ExecutionGateway::addMessageToSend(BrokerID id, const OrderReplaceMessage& msg)
{
    if (id >= brokers_.size())
        return false;

    auto& broker = brokers_[id];

    return broker.addMessageToSend(msg);
}

bool ExecutionGateway::addMessageToSend(BrokerID id, const OrderCancelMessage& msg)
{
    if (id >= brokers_.size())
        return false;

    auto& broker = brokers_[id];

    return broker.addMessageToSend(msg);
}