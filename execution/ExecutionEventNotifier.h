#ifndef EXECUTION_EVENT_NOTIFIER_H
#define EXECUTION_EVENT_NOTIFIER_H

#include <vector>
#include "Events.h"
#include "ExecutionEvents.h"

class ExecutionEventNotifier
{
public:
    void subscribeToOrderFailure(void* object, notifyFuncType func)
    {
        orderFailureListeners_.emplace_back(object, func);
    }
    void notifySingleOrderFailure(const SingleOrderFailureEvent& event)
    {
        for (auto& subscriber : orderFailureListeners_)
        {
            subscriber.notify(event);
        }
    }

    void notifyCancelOrderFailure(const CancelOrderFailureEvent& event)
    {
        for (auto& subscriber : orderFailureListeners_)
        {
            subscriber.notify(event);
        }
    }

    void notifyEditOrderFailure(const EditOrderFailureEvent& event)
    {
        for (auto& subscriber : orderFailureListeners_)
        {
            subscriber.notify(event);
        }
    }

private:
    std::vector<Subscriber> orderFailureListeners_;
};
#endif