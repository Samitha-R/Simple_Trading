#include <iostream>
#include <fstream>
#include <charconv>
#include <chrono>
#include <unordered_map>
#include "TypeDef.h"
#include "Buffer.h"
#include "StrategyEngine.h"
#include "BinaryLogger.h"
#include "FastRingBuffer.h"
#include "FixMessageParser.h"
#include "FixMessageBuilder.h"
#include "Session.h"
#include "Gateway.h"
#include "FeedHandler.h"
#include "FeedHandlerWrapper.h"
#include "Decimal.h"
#include "BrokerProfile.h"
#include "ExecutionEngine.h"
#include "MeanReversion.h"

int main() {

    /*BinaryLogger logmgr("system", 64);
    
    if (!logmgr.start())
        std::cout << "Log manager failed to start" << std::endl;

    for (int i = 0; i < 64; ++i) {
        char hostName[20];
        auto st = snprintf(hostName, 20,
                            "host %d", i);
        if (st < 0) {
            std::cout << "name generating failed" << std::endl;
        }
        StringMessage message;
        message.setMessage(hostName);
        logmgr.logMessage(message);
    }
    sleep(5);
    logmgr.stop();
    auto fileReader = logmgr.getLogFileReader("system_1.bin");

    if (fileReader) {
        auto typeMsgPair = fileReader->getNextMessage();

        while (typeMsgPair.first) {
            if (typeMsgPair.second == MessageType::STRING_MESSAGE) {
                auto conSuccessMsg = static_cast<StringMessage*>(typeMsgPair.first);
                std::cout << conSuccessMsg->getMessageSize() << " " << conSuccessMsg->getMessage() << std::endl;
            }
            typeMsgPair = fileReader->getNextMessage();
        }
    }*/

    /*FastRingBuffer<int> testbuffer(5);

    for (int i = 0; i < 6; ++i) {
        testbuffer.push(i);
    }

    auto bIter = testbuffer.begin();
    auto eIter = testbuffer.end();

    for ( ; bIter != eIter; bIter += 1) {
        std::cout << *bIter;
    }

    auto rbIter = testbuffer.rbegin();
    auto reIter = testbuffer.rend();

    for ( ; rbIter != reIter; rbIter += 1) {
        std::cout << *rbIter;
    }*/



    /*Test3::Liquidity bid(4, 100, true);
    bid.update(400, Test3::LiquidityInfo{4});;
    bid.update(500, Test3::LiquidityInfo{5});
    //auto l1 = bid.getBestLiquidity();
   // std::cout << l1.first << " " << l1.second << std::endl;
    std::cout << bid << std::endl;
    bid.update(200, Test3::LiquidityInfo{3});
    std::cout << bid << std::endl;
    bid.update(100, Test3::LiquidityInfo{1});
    std::cout << bid << std::endl;
    bid.update(600, Test3::LiquidityInfo{6});
   // auto l2 = bid.getBestLiquidity();
    //std::cout << l2.first << " " << l2.second << std::endl;
    std::cout << bid << std::endl;
    bid.update(1000, Test3::LiquidityInfo{10});
    std::cout << bid << std::endl;

    Test3::Liquidity bidcopy(2, 100, true);
    bid.copyTo(bidcopy);
    std::cout << bidcopy << std::endl; */

    /*Liquidity<LiquidityType::ASK> ask(4, 100);
    ask.update(700, LiquidityInfo{7});
    std::cout << ask << std::endl;
    ask.update(800, LiquidityInfo{8});
    std::cout << ask << std::endl;
    ask.update(900, LiquidityInfo{9});
    std::cout << ask << std::endl;
    ask.update(600, LiquidityInfo{6});
    std::cout << ask << std::endl;
    ask.update(500, LiquidityInfo{5});
    std::cout << ask << std::endl;
    ask.update(100, LiquidityInfo{1});
    std::cout << ask << std::endl;
    ask.update(500, LiquidityInfo{5});
    std::cout << ask << std::endl;

    LiquiditySnapshot<2, LiquidityType::ASK> askcopy;
    ask.getSnapshot(askcopy);
    std::cout << askcopy << std::endl; */

    /*char array[] = "8=FIX.4.4\x01"
                   "9=5\x01"
                   "35=0\x01"
                   "10=161\x01";*/

    /* char array[] = "8=FIX.4.4\x01"
                   "9=147\x01"
                   "35=X\x01"
                   "34=102\x01"
                   "49=EXCHANGE\x01"
                   "56=CLIENT\x01"
                   "52=20260310-09:30:15.123\x01"
                   "268=2\x01"
                   "279=0\x01"
                   "269=0\x01"
                   "55=AAPL\x01"
                   "270=190.40\x01"
                   "271=500\x01"
                   "290=1\x01"
                   "279=0\x01"
                   "269=2\x01"
                   "55=AAPL\x01"
                   "31=190.40\x01"
                   "32=100\x01"
                   "10=226\x01";*/

    /*char array[128] = "8=FIX.4.4\x01"
                "9=67\x01"
                "35=A\x01"
                "34=1\x01"
                "49=BROKER\x01"
                "56=CLIENT\x01"
                "52=20260318-10:15:23.456\x01"
                "98=0\x01"
                "108=30\x01"
                "10=136\x01";*/
    
    /*char array[128] = "8=FIX.4.4\x01"
                    "9=55\x01"
                    "35=0\x01"
                    "34=2\x01"
                    "49=BROKER\x01"
                    "56=CLIENT\x01"
                    "52=20260318-10:30:00.000\x01"
                    "10=069\x01";*/
    
   /* char snapshot[] = "8=FIX.4.4\x01"
"9=215\x01"
"35=W\x01"
"49=BROKER_MD\x01"
"56=samitha3\x01"
"34=0023\x01"
"52=20260621-12:30:01.105\x01"
"55=AAPL\x01"
"262=1\x01"
"83=884320\x01"
"268=4\x01"
"269=0\x01"
"270=420.10\x01"
"271=50\x01"
"290=1\x01"
"269=0\x01"
"270=420.05\x01"
"271=120\x01"
"290=2\x01"
"269=1\x01"
"270=420.15\x01"
"271=80\x01"
"290=1\x01"
"269=1\x01"
"270=420.20\x01"
"271=210\x01"
"290=2\x01"
"10=246\x01";

char array2[] = "8=FIX.4.4\x01"
"9=0103\x01"
"35=A\x01"
"49=samitha3\x01"
"56=samitha4\x01"
"34=0001\x01"
"52=20260309-05:16:43.134627\x01"
"553=samitha1\x01"
"554=samitha2\x01"
"98=0\x01"
"108=15\x01"
"10=170\x01";

    TradeSymbols symbols;
    symbols.addSymbol("AAPL");
    std::vector<SymbolID> interestedSymbols;
    interestedSymbols.push_back(symbols.getSymbolID("AAPL"));
    MessageParser parser(symbols, interestedSymbols);
    
    TagValueReader reader(snapshot, 0, 239, 255);
    auto &st = parser.parseHeader(reader);

    if (st.getType() == ParseStatus::Type::SUCCESS) {
        auto &success = static_cast<const ParseSuccess&>(st);
        auto &msg = success.getMessage();
        auto &header = static_cast<const FixMessageHeader&>(msg);
        std::cout << header;

        auto &st = parser.parseBody(reader);


        if (st.getType() == ParseStatus::Type::SUCCESS) {
            auto &success = static_cast<const ParseSuccess&>(st);
            auto &msg = success.getMessage();
            auto &snapshot = static_cast<const FixSnapshotMessage&>(msg);
            std::cout << snapshot << std::endl;
        }
    }*/

    /*MessageBuilder builder(2048, 1000,  MessageBuilder::TimeStampAccuracy::NANO);
    FixLogonMessage logon;
    logon.setUserName("samitha1");
    logon.setPassWord("samitha2");
    logon.setHeartBeatInterval(15);
    OutMessage outMessage;
    builder.addDataToOutMsg(logon, outMessage, "samitha3", "samitha4");
    builder.finalizeOutMessage(outMessage, 1);
    std::cout << outMessage;*/

    
    /*TradeSymbols symbols;
    symbols.addSymbol("AAPL");

    MessageBuilder builder(symbols, 2048, 1000,  MessageBuilder::TimeStampAccuracy::NANO);
    FixMarketDataRequest marketDataRequest;

    marketDataRequest.setRequestID(1);
    
    for (auto symbol : interestedSymbols) {
        marketDataRequest.addSymbol(symbol);
    }

    marketDataRequest.addEntryType(EntryType::Types::BID);
    marketDataRequest.addEntryType(EntryType::Types::OFFER);
    marketDataRequest.addEntryType(EntryType::Types::TRADE);
    marketDataRequest.setSubscriptionType(SubscriptionRequestType::Types::SNAPSHOT_AND_UPDATE);
    marketDataRequest.setUpdateType(UpdateType::Types::INCREMENTAL_REFRESH);
    OutMessage outMessage; 
    builder.addDataToOutMsg(marketDataRequest, outMessage, "samitha3", "samitha4");
    builder.finalizeOutMessage(outMessage, 1);
    std::cout << outMessage;*/

    /*TradeSymbols symbols;
    symbols.addSymbol("AAPL");
;
    BinaryLogger binaryLoger("system");

    if (!binaryLoger.start()) {
        std::cout << "Failed to start logger" << std::endl;
        return 1;
    }

    FeedHandlerConfig mdConfig("MDCLIENT", "MDSERVER", "localhost", 9051);
    mdConfig.setHeartBeatInterval(15);
    //SessionConfig<MessageParser, MessageBuilder> oeConfig("OECLIENT", "OESERVER", "localhost",5002);
    mdConfig.addInterestedSymbol(symbols.getSymbolID("AAPL"));
    MessageParser parser(symbols, mdConfig.getInterestedSymbols());
    MessageBuilder builder(symbols);

    Gateway<FeedHandler, FeedHandlerWrapper> mdGateway(1,0);
    //mdGateway.addSession(0, mdConfig, symbols, interestedSymbols);
    mdGateway.addSessionForMainThread(mdConfig, std::move(parser), std::move(builder), binaryLoger, symbols);
    mdGateway.start();*/

    /*Decimal<int,2> a(1);
    Decimal<int,2> b(2);
    std::cout << a + b << std::endl;*/

    /*std::vector<SymbolState> symbolStates(1, SymbolState(0));
    BrokerProfile brokerProfile(0, 5, 10, symbolStates);
    brokerProfile.setBalance(Price(1000000));
    brokerProfile.setMaxOrdersPerSec(10);
    brokerProfile.setMaxOrderSize(10);
    brokerProfile.setMinOrderSize(1);
    brokerProfile.setMaxPriceDeviation(0.1);
    std::cout << brokerProfile << std::endl;
    brokerProfile.handleEvent(SingleOrderEvent(0, 0, OrderSide::BUY, Price(100000), 5));
    std::cout << brokerProfile << std::endl;
    brokerProfile.handleEvent(SingleOrderEvent(0, 0, OrderSide::BUY, Price(100000), 5));
    std::cout << brokerProfile << std::endl;
    brokerProfile.handleEvent(CancelOrderEvent(0, 0, 1));
    std::cout << brokerProfile << std::endl;
    brokerProfile.handleEvent(CancelOrderAckEvent(0, 1));
    std::cout << brokerProfile << std::endl;*/
    /*brokerProfile.handleEvent(SingleOrderAckEvent(0, 1));
    std::cout << brokerProfile << std::endl;
    //brokerProfile.handleEvent(SingleOrderRejectEvent(0, 1));
    brokerProfile.handleEvent(OrderFillEvent(0, 1, Price(100000), 5, 5, 5));
    brokerProfile.handleEvent(OrderFillEvent(0, 1, Price(100000), 5, 10, 0));
    std::cout << brokerProfile << std::endl;*/

    /*TradeSymbols symbols;
    symbols.addSymbol("AAPL");
    Fix::MessageBuilder builder(symbols, 2048, 1000,  Fix::TimeStampAccuracy::NANO);
    NewOrderSingleMessage order(1, 0, OrderSide::BUY, std::chrono::system_clock::now(), 1, Price(100000), OrderType::LIMIT, TimeInForce::GOOD_TILL_CANCEL);
    OutMessage outMessage;
    builder.addDataToOutMsg(order, outMessage, "samitha3", "samitha4");
    builder.finalizeOutMessage(outMessage, 1);
    builder.printOutMessage(outMessage, std::cout);*/

    /*char execreport[] = "8=FIX.4.4\x01" //17 202
                "9=178\x01"
               "35=8\x01"
                "49=BROKER_GW\x01"
                "56=samitha3\x01"
                "34=201\x01"
                "52=20261026-06:40:14.102\x01"
                "37=BROKER_ORD_9988\x01"
                "11=1\x01"
                "17=EXEC_20261026_001\x01"
                "150=1\x01"
                "39=1\x01"
                "55=AAPL\x01"
                "54=1\x01"
                "38=100\x01"
                "14=40\x01"
                "151=60\x01"
                "31=10.0000\x01"
                "32=40\x01"
                "6=10.0000\x01"
                "10=108\x01";

    TradeSymbols symbols;
    symbols.addSymbol("AAPL");
    std::vector<SymbolID> interestedSymbols;
    interestedSymbols.push_back(symbols.getSymbolID("AAPL"));
    Fix::FixMessageParser parser(symbols, interestedSymbols);
    auto &st = parser.parseHeader(execreport, 0, 201,255);

    if (st.getType() == ParseStatus::Type::SUCCESS) {
        auto &success = static_cast<const ParseSuccess&>(st);
        auto &msg = success.getMessage();
        auto &header = static_cast<const MessageHeader&>(msg);
        std::cout << header << std::endl;

        auto &st = parser.parseBody();


        if (st.getType() == ParseStatus::Type::SUCCESS) {
            auto &success = static_cast<const ParseSuccess&>(st);
            auto &msg = success.getMessage();
            auto &snapshot = static_cast<const ExecutionReportMessage&>(msg);
            std::cout << snapshot << std::endl;
        }
    }*/

    /*TradeSymbols symbols;
    symbols.addSymbol("AAPL");
    Fix::MessageBuilder builder(symbols, 2048, 1000,  Fix::TimeStampAccuracy::NANO);
    OrderReplaceMessage order(2, 1, "order1", 0, OrderSide::BUY, std::chrono::system_clock::now(), 1, 1000000, OrderType::LIMIT, TimeInForce::GOOD_TILL_CANCEL);
    OutMessage outMessage;
    builder.addDataToOutMsg(order, outMessage, "samitha3", "samitha4");
    builder.finalizeOutMessage(outMessage, 1);
    builder.printOutMessage(outMessage, std::cout);*/

    /*TradeSymbols symbols;
    symbols.addSymbol("AAPL");
    Fix::MessageBuilder builder(symbols, 2048, 1000,  Fix::TimeStampAccuracy::NANO);
    OrderCancelMessage order(2, 1, "order1", 0, OrderSide::BUY, 1, std::chrono::system_clock::now());
    OutMessage outMessage;
    builder.addDataToOutMsg(order, outMessage, "samitha3", "samitha4");
    builder.finalizeOutMessage(outMessage, 1);
    builder.printOutMessage(outMessage, std::cout); */

    /*char orderCancelRejett[] = "8=FIX.4.4\x01"
                    "9=0133\x01"
                    "35=9\x01"
                    "49=samitha4\x01"
                    "56=samitha3\x01"
                    "34=0013\x01"
                    "52=20261101-08:00:00.110\x01"
                    "37=BRKR_99\x01"
                    "11=2\x01"
                    "41=1\x01"
                    "39=0\x01"
                    "434=2\x01"
                    "102=1\x01"
                    "58=Too late to cancel or replace\x01"
                    "10=075\x01";

                    
    TradeSymbols symbols;
    symbols.addSymbol("AAPL");
    std::vector<SymbolID> interestedSymbols;
    interestedSymbols.push_back(symbols.getSymbolID("AAPL"));
    Fix::FixMessageParser parser(symbols, interestedSymbols);
    auto &st = parser.parseHeader(orderCancelRejett, 0, 173, 255);

    if (st.getType() == ParseStatus::Type::SUCCESS) {
        auto &success = static_cast<const ParseSuccess&>(st);
        auto &msg = success.getMessage();
        auto &header = static_cast<const MessageHeader&>(msg);
        std::cout << header << std::endl;

        auto &st = parser.parseBody();

        if (st.getType() == ParseStatus::Type::SUCCESS) {
            auto &success = static_cast<const ParseSuccess&>(st);
            auto &msg = success.getMessage();
            auto &orderCancelReject = static_cast<const OrderCancelRejectMessage&>(msg);
            std::cout << orderCancelReject << std::endl;
        }
    }*/

    return 0;
}