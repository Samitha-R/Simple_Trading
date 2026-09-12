#include "ExecutionEngine.h"

void ExecutionEngine::exec()
{
    while (running_) {
        auto slot = eventQueue_.getReadSlot();
        if (slot) {
            auto& event = slot->getData();
            std::visit([this](auto&& evt) {

                }
            , event);
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
        case EventType::SINGLE_ORDER_ACK :
            slot->getData() = static_cast<const SingleOrderAckEvent&>(event);
            break;
        case EventType::SINGLE_ORDER_REJECT : 
            slot->getData() = static_cast<const SingleOrderRejectEvent&>(event);
            break;
        case EventType::CANCEL_ORDER :
            slot->getData() = static_cast<const CancelOrderEvent&>(event);
            break;
        case EventType::EDIT_ORDER :
            slot->getData() = static_cast<const EditOrderEvent&>(event);
            break;
        case EventType::ORDER_FILL :
            slot->getData() = static_cast<const OrderFillEvent&>(event);
            break;
        case EventType::CANCEL_ORDER_ACK :
            slot->getData() = static_cast<const CancelOrderAckEvent&>(event);
            break;
        case EventType::CANCEL_ORDER_REJECT :
            slot->getData() = static_cast<const CancelOrderRejectEvent&>(event);
            break;
        case EventType::EDIT_ORDER_ACK :
            slot->getData() = static_cast<const EditOrderAckEvent&>(event);
            break;
        case EventType::EDIT_ORDER_REJECT :
            slot->getData() = static_cast<const EditOrderRejectEvent&>(event);
            break;
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
