#include "StrategyEngine.h"
#include "ExecutionEvents.h"

void StrategyExecutor::exec()
{
    while (running_) {
        
         for (auto strategy : strategies_) {
            strategy->handleEvents();
         }
    }
}

void StrategyExecutor::stop()
{
    running_ = false;
    
    if (thread_.joinable())
        thread_.join();

}

bool StrategyExecutor::start()
{
    if (running_)
        return false;

    running_ = true;
    thread_ = std::jthread(&StrategyExecutor::exec, this);
    return true;
}

StrategyEngine::StrategyEngine(std::size_t numSymbols, std::size_t numThreads, std::size_t numStrategies) : strategies_(numSymbols), symbolvsStrategies_(numSymbols), threadExecutors_(numThreads)
{
    for (auto &strategies : symbolvsStrategies_) {
        strategies.reserve(2);
    }

}

void StrategyEngine::notify(const EventBase& event)
{
    auto eventType = event.getEventType();

    switch (eventType) {
        case EventType::MARKET_CHANGE: {
            const MarketChangeEvent& event = static_cast<const MarketChangeEvent&>(event);
            auto symbolID = event.getSymbolID();
            auto& interestedStrategies = symbolvsStrategies_[symbolID];

            for (auto strategy : interestedStrategies) {
                strategy->addEvent(event);
            }
            break;
        }
        case EventType::BROKER_STATUS: {
            const BrokerStatusEvent& event = static_cast<const BrokerStatusEvent&>(event);

            for (auto &strategy : strategies_) {
                strategy.addEvent(event);
            }
            break;
        }
        case EventType::SYMBOL_STATUS: {
            const SymbolStatusEvent& event = static_cast<const SymbolStatusEvent&>(event);
            auto symbolID = event.getSymbolID();
            auto& interestedStrategies = symbolvsStrategies_[symbolID];

            for (auto strategy : interestedStrategies) {
                strategy->addEvent(event);
            }

            break;
        }
        case EventType::ORDER_STATUS: {
            const OrderStatusEvent& event = static_cast<const OrderStatusEvent&>(event);
            auto strategyID = event.getStrategyID();
            strategies_[strategyID].addEvent(event);
            break;
        }
        case EventType::ORDER_REPLACE_FAILURE: {
            const OrderReplaceFailureEvent& event = static_cast<const OrderReplaceFailureEvent&>(event);
            auto strategyID = event.getStrategyID();
            strategies_[strategyID].addEvent(event);
            break;
        }
        default:
            break;
    }

}

bool StrategyEngine::start()
{
    if (running_)
        return false;

    running_ = true;

    for (auto &executor : threadExecutors_) {
        executor.start();
    }

    return true;
}

void StrategyEngine::stop()
{
    running_ = false;

    for (auto &executor : threadExecutors_) {
        executor.stop();
    }
}

void StrategyEngine::registerForOrderEvents(Subscriber subscriber)
{
    for (auto &strategy : strategies_) {
        strategy.registerForOrderEvents(subscriber);
    }
}


