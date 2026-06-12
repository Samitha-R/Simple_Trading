#include "StrategyEngine.h"

StrategyEngine::StrategyEngine(std::size_t numSymbols, std::size_t eventBufferSize): strategies_(numSymbols), marketEvents_(eventBufferSize)
{

}

void StrategyEngine::addMarket(MarketState &market)
{
    market.registerForMarketChanges(Subscriber(this, ::notify<StrategyEngine>));
}

void StrategyEngine::notify(const EventBase& event)
{
    auto type = event.getEventType();

    if (type == EventType::MARKET_CHANGE) {
        const MarketChangeEvent& marketEvent = static_cast<const MarketChangeEvent&>(event);
        marketEvents_.write(marketEvent);
    }
}

void StrategyEngine::exec()
{
    MarketChangeEvent event;

    while (run_) {
       bool success =  marketEvents_.read(event);

       if (success) {
            auto symbolID = event.getSymbolID();
            strategies_[symbolID].HandleEvent(event);
       }
    }
}

bool StrategyEngine::start()
{
    strategyThread_ = std::jthread(&StrategyEngine::exec, this);
    return true;
}

void StrategyEngine::stop()
{
    run_ = false;
    if (strategyThread_.joinable()) {
        strategyThread_.join();
    }
}


