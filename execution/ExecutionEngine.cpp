#include "ExecutionEngine.h"

ExecutionEngine::ExecutionEngine(std::size_t numSymbols, std::size_t numBrokers, std::size_t eventQueueSize) : eventQueue_(eventQueueSize)
{
    symbolStates_.reserve(numSymbols);

    for (auto i = 0; i < numSymbols; ++i) {
        symbolStates_.emplace_back(i);
    }

    brokerProfiles_.reserve(numBrokers);
    
}

bool ExecutionEngine::addBrokerProfile(std::size_t id, std::unique_ptr<BrokerProfile> brokerProfile)
{
    if (id >= brokerProfiles_.size()) {
        return false; // Invalid broker ID
    }
    brokerProfiles_[id] = std::move(brokerProfile);
    return true;
}

void ExecutionEngine::exec()
{
    auto forwardEvent = [this](const auto& event) {
        auto& brokerProfile = brokerProfiles_[event.getBrokerId()];

        if (brokerProfile) {
            brokerProfile->handleEvent(event);
        }
    };
    
    while (running_) {
        auto slot = eventQueue_.getReadSlot();
        if (slot) {
            auto& event = slot->getData();
            if (std::holds_alternative<SingleOrderEvent>(event)) {
                forwardEvent(std::get<SingleOrderEvent>(event));
            } else if (std::holds_alternative<CancelOrderEvent>(event)) {
                forwardEvent(std::get<CancelOrderEvent>(event));
            } else if (std::holds_alternative<EditOrderEvent>(event)) {
                forwardEvent(std::get<EditOrderEvent>(event));
            } else if (std::holds_alternative<ExecutionReportEvent>(event)) {
                forwardEvent(std::get<ExecutionReportEvent>(event));
            } else if (std::holds_alternative<OrderCancelRejectEvent>(event)) {
                forwardEvent(std::get<OrderCancelRejectEvent>(event));
            }
            eventQueue_.setReadComplete(slot);
        }
    }
}

bool ExecutionEngine::start()
{
    if (running_) {
        return false; // Already running
    }
    running_ = true;
    eventProcessingThread_ = std::jthread(&ExecutionEngine::exec, this);
    return true;
}

void ExecutionEngine::stop()
{
    if (!running_) {
        return; // Not running
    }
    running_ = false;
    if (eventProcessingThread_.joinable()) {
        eventProcessingThread_.join();
    }
}

void ExecutionEngine::notify(const EventBase& event)
{
    auto slot = eventQueue_.getWriteSlot();

    if (!slot) {
        // Handle the case where the queue is full, e.g., log a warning or drop the event
        return;
    }

    auto type = event.getEventType();

    switch(type) {
        case EventType::SINGLE_ORDER :
            slot->getData() = static_cast<const SingleOrderEvent&>(event);
            break;
        case EventType::CANCEL_ORDER :
            slot->getData() = static_cast<const CancelOrderEvent&>(event);
            break;
        case EventType::EDIT_ORDER :
            slot->getData() = static_cast<const EditOrderEvent&>(event);
            break;
        case EventType::EXECUTION_REPORT_EVENT :
            slot->getData() = static_cast<const ExecutionReportEvent&>(event);
            break;
        case EventType::ORDER_CANCEL_REJECT_EVENT :
            slot->getData() = static_cast<const OrderCancelRejectEvent&>(event);
        default:
            // Handle unknown event type, e.g., log an error
            break;
    }

    eventQueue_.setWriteComplete(slot);
}

void ExecutionEngine::registerForOrderManagerEvent(Subscriber subscriber)
{
    for (auto& brokerProfile : brokerProfiles_) {
        brokerProfile->registerForOrderManagerEvent(subscriber);
    }
}
