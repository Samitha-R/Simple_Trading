#ifndef EXECUTION_ENGINE_H
#define EXECUTION_ENGINE_H

#include <vector>
#include <thread>
#include "Events.h"
#include "CommonUtils.h"
#include "MWMRNoOverWriteSlotRingBuffer.h"
#include "ExecutionEvents.h"
#include "BrokerProfile.h"
#include "SymbolState.h"

using ComEventType = std::variant<SingleOrderEvent, SingleOrderAckEvent, SingleOrderRejectEvent, CancelOrderEvent, EditOrderEvent, OrderFillEvent, CancelOrderAckEvent, CancelOrderRejectEvent, EditOrderAckEvent, EditOrderRejectEvent>;

class ExecutionEngine
{
public:
    ExecutionEngine(std::size_t numSymbols, std::size_t numBrokers, std::size_t eventQueueSize) : eventQueue_(eventQueueSize),
        brokerProfiles_(numBrokers), symbolStates_(numSymbols) {}
    bool start();
    void stop();
    void notify(const EventBase& event);
    void registerForOrderManagerEvent(Subscriber subscriber);
private:
    void exec();
public:
    bool running_ = false;
    std::vector<std::unique_ptr<BrokerProfile>> brokerProfiles_;
    std::vector<std::unique_ptr<SymbolState>> symbolStates_;
    MWMRNoOverWriteSlotRingBuffer<ComEventType> eventQueue_;
    std::jthread eventProcessingThread_;
};

#endif