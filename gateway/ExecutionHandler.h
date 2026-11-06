#ifndef EXECUTION_HANDLER_H
#define EXECUTION_HANDLER_H

#include "Session.h"
#include "ExecutionEvents.h"

template<typename MsgParser, typename MsgBuilder, typename Logger> class ExecutionHandler : public Session<MsgParser, MsgBuilder, Logger>
{
public:
    ExecutionHandler(const SessionConfig& config, MsgParser messageParser, MsgBuilder messageBuilder, Logger& logger);
    void registerForExecutionEvents(Subscriber subscriber) { executionUpdateSubscribers_.push_back(subscriber); }
    void notify(const EventBase& event);
    void setBrokerID(BrokerID id) { brokerId_ = id; }
    BrokerID getBrokerID() { return brokerId_; }
    bool addMessageToSend(const NewOrderSingleMessage& msg) { return this->addMessageToSendTmp(msg); }
    bool addMessageToSend(const OrderReplaceMessage& msg) { return this->addMessageToSendTmp(msg);}
    bool addMessageToSend(const OrderCancelMessage& msg) { return this->addMessageToSendTmp(msg); }
private:
    void notifyExecutionUpdateSubscribers(const EventBase& event);
private:
    BrokerID brokerId_ = NoBrokerID;
    std::vector<Subscriber> executionUpdateSubscribers_;
};

template<typename MsgParser, typename MsgBuilder, typename Logger> ExecutionHandler<MsgParser, MsgBuilder, Logger>::
    ExecutionHandler(const SessionConfig& config, MsgParser messageParser, MsgBuilder messageBuilder, Logger& logger) :
    Session<MsgParser, MsgBuilder, Logger>(config, messageParser, messageBuilder, logger)
{
    executionUpdateSubscribers_.reserve(3);
    this->registerForSessionEvents(this);
}

template<typename MsgParser, typename MsgBuilder, typename Logger> void ExecutionHandler<MsgParser, MsgBuilder, Logger>::notifyExecutionUpdateSubscribers(const EventBase& event)
{
    for (auto subscriber : executionUpdateSubscribers_) {
        subscriber.notify(event);
    }
}

template<typename MsgParser, typename MsgBuilder, typename Logger> void ExecutionHandler<MsgParser, MsgBuilder, Logger>::notify(const EventBase& event)
{
    if (event.getEventType() == EventType::NEW_FIX_MESSAGE) {
        auto& msg = static_cast<const NewFixMessageEvent&>(event).getMessage();
        auto type = msg.getMessageType();

        if (type == MessageType::EXECUTION_REPORT) {
            auto& execMsg = static_cast<const ExecutionReportMessage&>(msg);
            ExecutionReportEvent event(brokerId_, execMsg);
            notifyExecutionUpdateSubscribers(event);
        } else if (type == MessageType::ORDER_CANCEL_REJECT) {
            auto& rejectMsg = static_cast<const OrderCancelRejectMessage&>(msg);
            OrderCancelRejectEvent event(brokerId_, rejectMsg);
            notifyExecutionUpdateSubscribers(event);
        }
    }
}

#endif