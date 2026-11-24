#include "StrategyEngine.h"
#include "BinaryLogger.h"
#include "FixMessageParser.h"
#include "FixMessageBuilder.h"
#include "Gateway.h"
#include "FeedHandler.h"
#include "Decimal.h"
#include "BrokerProfile.h"
#include "ExecutionEngine.h"
#include "MeanReversion.h"

int main() {

    auto numBrokers = 1;

    BinaryLogger binaryLoger("system");

    if (!binaryLoger.start()) {
        std::cout << "Failed to start logger" << std::endl;
        return 1;
    }

    TradeSymbols symbols;
    symbols.addSymbol("AAPL");

    FeedHandlerConfig mdConfig("MDCLIENT", "MDSERVER", "localhost", 9051);
    mdConfig.setHeartBeatInterval(15);
    mdConfig.addInterestedSymbol(symbols.getSymbolID("AAPL"));
    Fix::MessageBuilder mdBuilder(symbols);
    Fix::FixMessageParser mdParser(symbols, mdConfig.getInterestedSymbols());
    MarketGateway mdGateway(numBrokers, 1);
    mdGateway.addSession(0, mdConfig, std::move(mdParser), std::move(mdBuilder), binaryLoger, symbols);


    SessionConfig exConfig("EXCLIENT", "EXSERVER", "localhost", 9052);
    ExecutionGateway exGateway(numBrokers,0);
    exConfig.setHeartBeatInterval(15);
    Fix::MessageBuilder exBuilder(symbols);
    Fix::FixMessageParser exParser(symbols, mdConfig.getInterestedSymbols());
    exGateway.addSessionForMainThread(exConfig, std::move(exParser), std::move(exBuilder), binaryLoger);

    StrategyEngine strategyEngine(symbols.getNumSymbols(), numBrokers, 1, 1);
    auto aaplStrategy = std::make_unique<MeanReversion>(1024, 0, symbols.getSymbolID("AAPL"), 1024, 1024, 100000, 20000);
    strategyEngine.addStrategy(std::move(aaplStrategy), 0, {symbols.getSymbolID("AAPL")}, {0});

    ExecutionEngine executionEngine(symbols.getNumSymbols(), numBrokers, 1024,  exGateway);
    auto brokerProfile = executionEngine.addBrokerProfile(0, 200, 10000);
    brokerProfile->setBalance(Price(1000));
    brokerProfile->setMaxOrdersPerSec(100);
    mdGateway.registerForMarketEvents(&strategyEngine);
    exGateway.registerForExecutionEvents(&executionEngine);
    strategyEngine.registerForOrderEvents(&executionEngine);
    executionEngine.registerForOrderManagerEvent(&strategyEngine);

    strategyEngine.start();
    executionEngine.start();
    mdGateway.start();
    exGateway.start();


    return 0;
}
