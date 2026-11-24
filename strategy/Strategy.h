#ifndef STRATEGY_H
#define STRATEGY_H

#include <variant>
#include "Events.h"
#include "MWMRNoOverWriteSlotRingBuffer.h"
#include "MarketState.h"
#include "FeedHandlerEvents.h"
#include "ExecutionEvents.h"

using CommonEventHolder = std::variant<BookUpdateEventL10, TradeEvent, BrokerStatusEvent, SymbolStatusEvent, OrderStatusEvent, OrderReplaceFailureEvent>;

class StrategyBase
{
public:
    void registerForOrderEvents(Subscriber subscriber) {  orderEventListeners_.push_back(subscriber); }
protected :
    StrategyBase(std::size_t eventQueueSize) : strategyID_(nextStrategyID_++), events_(eventQueueSize) { orderEventListeners_.reserve(3); }
    template<typename T> void notifyOrderEventListners(const T& event);
public:
    bool addEvent(const BookUpdateEventL10& event);
    bool addEvent(const TradeEvent& event);
    bool addEvent(const BrokerStatusEvent& event);
    bool addEvent(const SymbolStatusEvent& event);
    bool addEvent(const OrderStatusEvent& event);
    bool addEvent(const OrderReplaceFailureEvent& event);
    StrategyID getStrategyID() const { return strategyID_; }
private:
    template<typename T> bool addEventTemp(const T& event);
protected:
    StrategyID strategyID_ = NoStrategyID;
    MWMRNoOverWriteSlotRingBuffer<CommonEventHolder> events_;
    std::vector<Subscriber> orderEventListeners_;
private:
    static StrategyID nextStrategyID_;
};

class StrategyWrapper
{
public:
    template<typename T> StrategyWrapper(std::unique_ptr<T> strategy);
    StrategyWrapper()  = default;
    StrategyWrapper(const StrategyWrapper&) = delete;
    StrategyWrapper(StrategyWrapper&&) = default;
    StrategyWrapper& operator=(const StrategyWrapper&) = delete;
    StrategyWrapper& operator=(StrategyWrapper&&) = default;
    
    bool addEvent(const BookUpdateEventL10& event) {
        return addBookUpdateEventL10Fn_(strategyPtr_, event);
    }

    bool addEvent(const TradeEvent& event) {
        return addTradeEventFn_(strategyPtr_, event);
    }

    bool addEvent(const BrokerStatusEvent& event) {
        return addBrokerStatusEventFn_(strategyPtr_, event);
    }

    bool addEvent(const SymbolStatusEvent& event) {
        return addSymbolStatusEventFn_(strategyPtr_, event);
    }

    bool addEvent(const OrderStatusEvent& event) {
        return addOrderStatusEventFn_(strategyPtr_, event);
    }

    bool addEvent(const OrderReplaceFailureEvent& event) {
        return addOrderReplaceFailureEventFn_(strategyPtr_, event);
    }

    void handleEvents() {
        handleEventsFn_(strategyPtr_);
    }

    void registerForOrderEvents(Subscriber subscriber) {
        regsiterForOrderEventsFn_(strategyPtr_, subscriber);
    }
    ~StrategyWrapper() { deleteFn_(strategyPtr_); }
private:
    bool(*addBookUpdateEventL10Fn_)(void*, const BookUpdateEventL10&)  = nullptr;
    bool(*addTradeEventFn_)(void*, const TradeEvent&) = nullptr;
    bool(*addBrokerStatusEventFn_)(void*, const BrokerStatusEvent&)  = nullptr;
    bool(*addSymbolStatusEventFn_)(void*, const SymbolStatusEvent&)  = nullptr;
    bool(*addOrderStatusEventFn_)(void*, const OrderStatusEvent&)  = nullptr;
    bool(*addOrderReplaceFailureEventFn_)(void*, const OrderReplaceFailureEvent&)  = nullptr;
    void(*handleEventsFn_ )(void*)= nullptr;
    void(*deleteFn_)(void*) = nullptr;
    void(*regsiterForOrderEventsFn_)(void*, Subscriber)   = nullptr;
    void* strategyPtr_ = nullptr;
};

template<typename T> void StrategyBase::notifyOrderEventListners(const T& event)
{
    for (auto& subscriber : orderEventListeners_) {
        subscriber.notify(event);
    }
}

template<typename T> bool StrategyBase::addEventTemp(const T& event)
{
    auto slot = events_.getWriteSlot();

    if (!slot)
        return false;

    slot->getData() = event;
    events_.setWriteComplete(slot);
    return true; 
}

template<typename T> StrategyWrapper::StrategyWrapper(std::unique_ptr<T> strategy)
{
    strategyPtr_ = strategy.release();

    addBookUpdateEventL10Fn_ = [](void* ptr, const BookUpdateEventL10& event) -> bool {
        T* strategy = static_cast<T*>(ptr);
        return strategy->addEvent(event);
    };

    addTradeEventFn_ = [](void* ptr, const TradeEvent& event) -> bool {
        T* strategy = static_cast<T*>(ptr);
        return strategy->addEvent(event);
    };

    addBrokerStatusEventFn_ = [](void* ptr, const BrokerStatusEvent& event) -> bool {
        T* strategy = static_cast<T*>(ptr);
        return strategy->addEvent(event);
    };

    addSymbolStatusEventFn_ = [](void* ptr, const SymbolStatusEvent& event) -> bool {
        T* strategy = static_cast<T*>(ptr);
        return strategy->addEvent(event);
    };

    addOrderStatusEventFn_ = [](void* ptr, const OrderStatusEvent& event) -> bool {
        T* strategy = static_cast<T*>(ptr);
        return strategy->addEvent(event);
    };

    addOrderReplaceFailureEventFn_ = [](void* ptr, const OrderReplaceFailureEvent& event) -> bool {
        T* strategy = static_cast<T*>(ptr);
        return strategy->addEvent(event);
    };

    handleEventsFn_ = [](void* ptr) {
        T* strategy = static_cast<T*>(ptr);
        strategy->handleEvents();
    };

    regsiterForOrderEventsFn_ = [] (void* ptr, Subscriber subscriber) {
        T* strategy = static_cast<T*>(ptr);
        strategy->registerForOrderEvents(subscriber);
    };

    deleteFn_ = [](void* ptr) {
        T* strategy = static_cast<T*>(ptr);
        delete strategy;
    };
}


#endif