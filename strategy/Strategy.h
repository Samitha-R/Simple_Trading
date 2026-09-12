#ifndef STRATEGY_H
#define STRATEGY_H

#include <variant>
#include "Events.h"
#include "MWMRNoOverWriteSlotRingBuffer.h"
#include "MarketState.h"
#include "FeedHandlerEvents.h"


template<typename T> concept StrategyHandleEvents = std::same_as<T, MarketChangeEvent>;
using CommonEventHolder = std::variant<MarketChangeEvent>;

class StrategyBase
{
public:
    void registerForOrderEvents(Subscriber subscriber) {  orderEventListeners_.push_back(subscriber); }
protected :
    StrategyBase(std::size_t eventQueueSize) : events_(eventQueueSize) { orderEventListeners_.reserve(3); }
    template<typename T> void notifyOrderEventListners(const T& event) const;
public:
    bool addMarketChangeEvent(const MarketChangeEvent& event);
protected:
    MWMRNoOverWriteSlotRingBuffer<CommonEventHolder> events_;
    std::vector<Subscriber> orderEventListeners_;
};

class StrategyWrapper
{
private:
    using StrategyDeleteFunction = void(*)(void*);
    using AddEventFunctionType =  bool(*)(void*, const MarketChangeEvent&);
    using HandleEventFunctionType = void(*)(void*);
    using DeleteFunctionType = void(*)(void*);
    using RegisterForOrderEventsFunctionType = void(*)(void*, Subscriber);
public:
    template<typename T> StrategyWrapper(std::unique_ptr<T> strategy);
    StrategyWrapper()  = default;
    StrategyWrapper(const StrategyWrapper&) = delete;
    StrategyWrapper(StrategyWrapper&&) = default;
    StrategyWrapper& operator=(const StrategyWrapper&) = delete;
    StrategyWrapper& operator=(StrategyWrapper&&) = default;
    bool addMarketChangeEvent(const MarketChangeEvent& event) {
        return addMarketChangeEventFn_(strategyPtr_, event);
    }
    void handleEvents() {
        handleEventsFn_(strategyPtr_);
    }

    void registerForOrderEvents(Subscriber subscriber) {
        regsiterForOrderEventsFn_(strategyPtr_, subscriber);
    }
    ~StrategyWrapper() { deleteFn_(strategyPtr_); }
private:
    AddEventFunctionType addMarketChangeEventFn_ = nullptr;
    HandleEventFunctionType handleEventsFn_ = nullptr;
    DeleteFunctionType deleteFn_ = nullptr;
    RegisterForOrderEventsFunctionType  regsiterForOrderEventsFn_ = nullptr;
    void* strategyPtr_ = nullptr;
};

template<typename T> StrategyWrapper::StrategyWrapper(std::unique_ptr<T> strategy)
{
    strategyPtr_ = strategy.release();
    addMarketChangeEventFn_ = [](void* ptr, const EventBase& event) -> bool {
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