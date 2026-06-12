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
#include "ExecutionEventNotifier.h"

using ComEventType = std::variant<SingleOrderEvent, SingleOrderAckEvent, SingleOrderRejectEvent, CancelOrderEvent, EditOrderEvent, OrderFillEvent, CancelOrderAckEvent, CancelOrderRejectEvent, EditOrderAckEvent, EditOrderRejectEvent>;

class ExecutionEngine : public ExecutionEventNotifier
{
public:
    ExecutionEngine(std::size_t numSymbols, std::size_t numBrokers, std::size_t eventQueueSize) : eventQueue_(eventQueueSize),
        brokerProfiles_(numBrokers), symbolStates_(numSymbols) {}
    bool start();
    void stop();
    template<typename T> void notify(const T& event);
private:
    void exec();
public:
    bool running_ = false;
    std::vector<std::unique_ptr<BrokerProfile>> brokerProfiles_;
    std::vector<std::unique_ptr<SymbolState>> symbolStates_;
    MWMRNoOverWriteSlotRingBuffer<ComEventType> eventQueue_;
    std::jthread eventProcessingThread_;
};

template<typename T> void ExecutionEngine::notify(const T& event)
{
    auto slot = eventQueue_.getWriteSlot();

    if (!slot) {
        // Handle the case where the queue is full, e.g., log a warning or drop the event
        return;
    }

    slot->getData() = event;
    eventQueue_.setWriteComplete(slot);
}

#endif