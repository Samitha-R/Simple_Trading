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
