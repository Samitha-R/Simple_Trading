#ifndef EXECUTION_HANDLER_WRAPPER_H
#define EXECUTION_HANDLER_WRAPPER_H


#include "SessionWrapper.h"

class ExecutionHandlerWrapper : public SessionWrapper
{
public:
    template<typename ExecutionHandler>  ExecutionHandlerWrapper(std::unique_ptr<ExecutionHandler> executionHandler);
    void registerForExecutionEvents(Subscriber subscriber) {  registerForExecutionEvents_(session_, subscriber); }
    bool addMessageToSend(const NewOrderSingleMessage& msg) { return sendSingleOrder_(session_, msg); }
    bool addMessageToSend(const OrderReplaceMessage& msg) { return sendOrderReplace_(session_, msg);}
    bool addMessageToSend(const OrderCancelMessage& msg) { return sendOrderCancel_(session_, msg); }
private:
    void(*registerForExecutionEvents_)(void*, Subscriber subscriber);
    bool(*sendSingleOrder_)(void*, const NewOrderSingleMessage&) = nullptr;
    bool(*sendOrderReplace_)(void*, const OrderReplaceMessage&) = nullptr;
    bool(*sendOrderCancel_)(void*, const OrderCancelMessage&) = nullptr;
};

 template<typename ExecutionHandler> ExecutionHandlerWrapper::ExecutionHandlerWrapper(std::unique_ptr<ExecutionHandler> executionHandler) : SessionWrapper(std::move(executionHandler))
 {
    registerForExecutionEvents_ = [](void* executionHandler, Subscriber subscriber) {
        static_cast<ExecutionHandler*>(executionHandler)->registerForExecutionEvents(subscriber);
    };

    sendSingleOrder_ = [](void* executionHandler, const NewOrderSingleMessage& msg)->auto{
        return static_cast<ExecutionHandler*>(executionHandler)->addMessageToSend(msg);
    };

    sendOrderReplace_ = [](void* executionHandler, const OrderReplaceMessage& msg)->auto{
        return static_cast<ExecutionHandler*>(executionHandler)->addMessageToSend(msg);
    };

    sendOrderCancel_ = [] (void* executionHandler, const OrderCancelMessage& msg)->auto {
        return static_cast<ExecutionHandler*>(executionHandler)->addMessageToSend(msg);
    };
 }

#endif