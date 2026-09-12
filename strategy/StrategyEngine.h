#ifndef STRATEGY_ENGINE_H
#define STRATEGY_ENGINE_H

#include <vector>
#include <thread>
#include "TypeDef.h"
#include "Strategy.h"

class StrategyExecutor
{
public:
    StrategyExecutor()  { strategies_.resize(3); }
public:
    void addStrategy(StrategyWrapper* strategy) { strategies_.push_back(strategy);}
    void stop();
    bool start();
    void exec();
private:
    bool running_ = false;
    std::jthread thread_;
    std::vector<StrategyWrapper*> strategies_;
};

class StrategyEngine
{
public:
    StrategyEngine(std::size_t numSymbols, std::size_t numThreads);
    template<typename T> void addStrategy(std::unique_ptr<T> StrategyPtr, std::size_t threadID, const std::initializer_list<SymbolID>& symbols);
    void notify(const EventBase& event);
    bool start();
    void stop();
    void registerForOrderEvents(Subscriber subscriber);
private:
    void exec();
private:
    bool running_ = false;
    std::vector<StrategyWrapper> strategies_;
    std::vector<std::vector<StrategyWrapper*>> symbolvsStrategies_;
    std::vector<StrategyExecutor> threadExecutors_;
    
    
};

template<typename T> void StrategyEngine::addStrategy(std::unique_ptr<T> strategyPtr, std::size_t threadID, const std::initializer_list<SymbolID>& symbols)
{
    auto currentIndex = strategies_.size();
    strategies_.emplace_back(strategyPtr);
    threadExecutors_[threadID].addStrategy(&strategies_[currentIndex]);

    for (auto symbolId : symbols) {
        symbolvsStrategies_[symbolId].push_back(&strategies_[currentIndex]);
    }
}


#endif