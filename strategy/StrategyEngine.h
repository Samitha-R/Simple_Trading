#ifndef STRATEGY_ENGINE_H
#define STRATEGY_ENGINE_H

#include <vector>
#include <thread>
#include "Events.h"
#include "SWSRRingBuffer.h"
#include "MarketState.h"
#include "FeedHandlerEvents.h"

using StrategyResolveFunction = void(*) (void*, const MarketChangeEvent& event);
using StrategyDeleteFunction = void(*)(void*);

template<typename T> void strategyResolve(void *strategyPtr, const MarketChangeEvent& event)
{
    T* strategy = static_cast<T*>(strategyPtr);
    strategy->handleEvent(event); 
}

template<typename T> void strategyDelete(void *strategyPtr)
{
    T* strategy = static_cast<T*>(strategyPtr);
    delete strategy; 
}

class StrategyWrapper
{
public:
    StrategyWrapper() = default;
    void HandleEvent(const MarketChangeEvent& event) {
        if (callFn_ && strategyPtr_)
            callFn_(strategyPtr_, event);
    }
    bool isValid() const { return callFn_ != nullptr && strategyPtr_ != nullptr; }  
    StrategyWrapper(StrategyResolveFunction callFn, StrategyDeleteFunction deleteFn, void* strategyPtr) : callFn_(callFn), deleteFn_(deleteFn), strategyPtr_(strategyPtr) {}
    ~StrategyWrapper() { deleteFn_(strategyPtr_); }
private:
    StrategyResolveFunction callFn_ = nullptr;
    StrategyDeleteFunction deleteFn_ = nullptr;
    void* strategyPtr_ = nullptr;
};

class StrategyEngine
{
public:
    StrategyEngine(std::size_t numSymbols, std::size_t eventBufferSize);
    void setStrategy(SymbolID id, StrategyWrapper strategy) { strategies_[id] = strategy; }
    void notify(const EventBase& event);
    bool start();
    void stop();
private:
    void exec();
private:
    std::vector<StrategyWrapper> strategies_;
    SWSRRingBuffer<MarketChangeEvent> marketEvents_;
    bool run_ = true;
    std::jthread strategyThread_; 
};



#endif