#ifndef FEEDHANDLER_WRAPPER_H
#define FEEDHANDLER_WRAPPER_H

#include "SessionWrapper.h"
#include  "MarketState.h"

class FeedHandlerWrapper : public SessionWrapper
{
public:
    template<typename FeedHandler> FeedHandlerWrapper(std::unique_ptr<FeedHandler> feedHandler);
    void registerForMarketEvents(Subscriber subscriber) {  registerForMarketEvents_(session_, subscriber); }
    SymbolMarketState* getSymbolMarketState(SymbolID symbolId) { return getSymbolMarketState_(session_, symbolId); }
    bool addMessageToSend(const MarketDataRequestMessage& msg) { return sendMDRquest_ (session_, msg); }
    
private:
    void(*registerForMarketEvents_)(void*, Subscriber subscriber) = nullptr;
    SymbolMarketState*(*getSymbolMarketState_)(void*, SymbolID symbolId) = nullptr;
    bool(*sendMDRquest_)(void*, const MarketDataRequestMessage&) = nullptr;
};

 template<typename FeedHandler> FeedHandlerWrapper::FeedHandlerWrapper(std::unique_ptr<FeedHandler> feedHandler) : SessionWrapper(std::move(feedHandler))
 {
    registerForMarketEvents_ = [](void* feedHandler, Subscriber subscriber) {
        static_cast<FeedHandler*>(feedHandler)->registerForMarketEvents(subscriber);
    };

    getSymbolMarketState_ = [](void* feedHandler, SymbolID symbolId) {
        return static_cast<FeedHandler*>(feedHandler)->getSymbolMarketState(symbolId);
    };
    
    sendMDRquest_ = [](void* feedHandler, const MarketDataRequestMessage& msg)->auto{
        return static_cast<FeedHandler*>(feedHandler)->addMessageToSend(msg);
    };

 }
#endif