#include <iostream>
#include <fstream>
#include <charconv>
#include <chrono>
#include <unordered_map>
#include "TypeDef.h"
#include "Buffer.h"
#include "L2Book.h"
#include "StrategyEngine.h"
#include "BinaryLogger.h"
#include "FastRingBuffer.h"
#include "MessageParser.h"
#include "MessageBuilder.h"
#include "Session.h"
#include "Gateway.h"
#include "FeedHandler.h"
#include "FeedHandlerWrapper.h"

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

   /* auto printLiquidity = [](Liquidity &bid)
    {
        std::cout << std::endl;

        auto ib = bid.begin();
        auto ie = bid.end();

        for ( ; ie != ib; ib += 1) {
            std:: cout << *ib << " ";
        }

        auto bestPrice = bid.getBestLiquidity();
        std::cout << std::endl;
        std::cout <<  "BestPrice: " << bestPrice.first << " BestVolume: " << bestPrice.second << " best price changeed: " << bid.isBestChangeWithLastUpdate();
    };

    BidLiquidity bid(4, 0.01);
    bid.update(0.04, 4);
    printLiquidity(bid);
    bid.update(0.05, 5);
    printLiquidity(bid);
    bid.update(0.02, 3);
    printLiquidity(bid);
    bid.update(0.01, 1);
    printLiquidity(bid);
    bid.update(0.06, 6);
    printLiquidity(bid);
    bid.update(0.10, 10);
    printLiquidity(bid);

    BidLiquidity bidcopy(2, 0.01);
    bidcopy.init();
    bid.copyTo(bidcopy);
    printLiquidity(bidcopy);

    
    AskLiquidity ask(4, 0.01);
    ask.update(0.04, 4);
    printLiquidity(ask);
    ask.update(0.05, 5);
    printLiquidity(ask);
    ask.update(0.06, 6);
    printLiquidity(ask);
    ask.update(0.03, 3);
    printLiquidity(ask);
    ask.update(0.02, 2);
    printLiquidity(ask);
    ask.update(0.01, 1);
    printLiquidity(ask);
    ask.update(0.05, 5);
    printLiquidity(ask);

    AskLiquidity askcopy(2, 0.01);
    askcopy.init();
    ask.copyTo(askcopy);
    printLiquidity(askcopy);*/

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

    char array[128] = "8=FIX.4.4\x01"
                "9=67\x01"
                "35=A\x01"
                "34=1\x01"
                "49=BROKER\x01"
                "56=CLIENT\x01"
                "52=20260318-10:15:23.456\x01"
                "98=0\x01"
                "108=30\x01"
                "10=136\x01";
    
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

    TradeSymbols symbols;
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
    mdGateway.start();

    return 0;
}
