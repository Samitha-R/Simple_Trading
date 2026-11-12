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
    StrategyEngine(std::size_t numSymbols, std::size_t numThreads, std::size_t numStrategies);
    template<typename T> bool addStrategy(std::unique_ptr<T> StrategyPtr, std::size_t threadID, const std::initializer_list<SymbolID>& symbols);
    void notify(const EventBase& event);
    bool start();
    void stop();
    void registerForOrderEvents(Subscriber subscriber);
private:
    bool running_ = false;
    std::vector<StrategyWrapper> strategies_;
    std::vector<std::vector<StrategyWrapper*>> symbolvsStrategies_;
    std::vector<StrategyExecutor> threadExecutors_;
};

template<typename T> bool StrategyEngine::addStrategy(std::unique_ptr<T> strategyPtr, std::size_t threadID, const std::initializer_list<SymbolID>& symbols)
{
    auto strategyID = strategyPtr->getStrategyID();

    if (strategyID >= strategies_.size()) {
        return false;
    }

    strategies_[strategyID] = StrategyWrapper(std::move(strategyPtr));
    threadExecutors_[threadID].addStrategy(&strategies_[strategyID]);

    for (auto symbolId : symbols) {
        symbolvsStrategies_[symbolId].push_back(&strategies_[strategyID]);
    }
    return true;
}


#endif