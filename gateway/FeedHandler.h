#ifndef FEED_HANDLER_H
#define FEED_HANDLER_H

#include <map>
#include "Session.h"
#include "MarketState.h"
#include "FeedHandlerEvents.h"
#include "FeedHandlerWrapper.h"

class FeedHandlerConfig :  public SessionConfig
{
public:
    FeedHandlerConfig(std::string_view id, std::string_view targetId, std::string_view host, int port) : SessionConfig(id, targetId, host, port) {}
    void addInterestedSymbol(SymbolID symbolId) { interestedSymbols_[symbolId] = SymbolMarketStateConfig(); }
    void addInterestedSymbol(SymbolID symbolId, const SymbolMarketStateConfig& config) { interestedSymbols_[symbolId] = config; }
    const std::map<SymbolID, SymbolMarketStateConfig>& getInterestedSymbolConfig() const { return interestedSymbols_; }
    inline std::vector<SymbolID> getInterestedSymbols() const;
private:
   std::map<SymbolID, SymbolMarketStateConfig> interestedSymbols_;
};

std::vector<SymbolID> FeedHandlerConfig::getInterestedSymbols() const
{
    std::vector<SymbolID> symbols;
    symbols.reserve(interestedSymbols_.size());

    for (auto& symbolConfig : interestedSymbols_) {
        symbols.push_back(symbolConfig.first);
    }
    return symbols;
}

template<typename MsgParser, typename MsgBuilder, typename Logger> class FeedHandler : public Session<MsgParser, MsgBuilder, Logger>
{
    using Wrapper = FeedHandlerWrapper;
public:
    FeedHandler(const FeedHandlerConfig& config, MsgParser messageParser, MsgBuilder messageBuilder, Logger& logger, const TradeSymbols &symbols);
public:
    void setBrokerID(BrokerID id) { brokerId_ = id; }
    BrokerID getBrokerID() { return brokerId_; }
    void registerForMarketEvents(Subscriber subscriber) {  marketUpdateSubscribers_.push_back(subscriber); }
    SymbolMarketState* getSymbolMarketState(SymbolID symbolId) { return symbolMarketStates_[symbolId].get(); }
private:
    void handleSessionEvents(const EventBase& event);
    void notifyMarketChanges(const MarketChangeEvent& event);
private:
    BrokerID brokerId_ = NoBrokerID;
    std::size_t mdReqID_ = 0;
    std::vector<SymbolID> interestedSymbols_;
    std::vector<std::unique_ptr<SymbolMarketState>> symbolMarketStates_;
    std::vector<char> symbolChangeStatus_;
    std::vector<Subscriber> marketUpdateSubscribers_;
};

template<typename MsgParser, typename MsgBuilder, typename Logger> FeedHandler<MsgParser, MsgBuilder, Logger>::FeedHandler(const FeedHandlerConfig& config, MsgParser messageParser, MsgBuilder messageBuilder, Logger& logger, const TradeSymbols &symbols) : Session<MsgParser, MsgBuilder, Logger>(config, messageParser, messageBuilder, logger),
                                                                                                  interestedSymbols_(config.getInterestedSymbols()),
                                                                                                  symbolMarketStates_(symbols.getNumSymbols()),
                                                                                                  symbolChangeStatus_(symbols.getNumSymbols(), 0) 
{
    auto& symbolConfigs = config.getInterestedSymbolConfig();

    for (auto& symbolConfig : symbolConfigs) {
        symbolMarketStates_[symbolConfig.first] = std::make_unique<SymbolMarketState>(symbolConfig.second);
    }

    Subscriber callback(this, [](void *sub, const EventBase& event) { static_cast<FeedHandler<MsgParser, MsgBuilder, Logger>*>(sub)->handleSessionEvents(event);});
    this->registerForSessionEvents(callback);
}

template<typename MsgParser, typename MsgBuilder, typename Logger> void FeedHandler<MsgParser, MsgBuilder, Logger>::handleSessionEvents(const EventBase& event)
{
    if (event.getEventType() == EventType::NEW_FIX_MESSAGE) {
        auto &msg = static_cast<const NewFixMessageEvent&>(event).getMessage();
        auto type = msg.getMessageType();

        if (type == FixMessageType::MARKET_DATA) {
            auto &marketDataMsg = static_cast<const FixMarketDataMessage&>(msg);

            for (int i = 0; i < symbolMarketStates_.size(); ++i) {
                symbolChangeStatus_[i] = false;

                if (!symbolMarketStates_[i])
                    continue;

                auto& symbolMarketData = marketDataMsg.getFixMarketData(i);

                if (symbolMarketData.empty())
                    continue;

                auto status = symbolMarketStates_[i]->addUpdate(symbolMarketData);

                if (status == LiquidityUpdateStatus::SUCCESSFULL) {
                    symbolChangeStatus_[i] = true;
                } else if (status == LiquidityUpdateStatus::MISSING_UPDATE) {
                        //reset
                    std::fill(symbolChangeStatus_.begin(), symbolChangeStatus_.end(), false);
                    symbolMarketStates_.clear();
                    FixMarketDataRequest marketDataRequest(1);

                    marketDataRequest.addSymbol(i);
                    marketDataRequest.setRequestID(++mdReqID_);
                    marketDataRequest.addEntryType(EntryType::Types::BID);
                    marketDataRequest.addEntryType(EntryType::Types::OFFER);
                    marketDataRequest.setSubscriptionType(SubscriptionRequestType::Types::SNAPSHOT);
        
                    if (!this->addMessageToSend(marketDataRequest)) {
                        MarketDataRequestSendingFailed log(this->getSessionID());
                        this->getLogger().logMessage(log);
                        return;
                    }

                } else {
                    symbolChangeStatus_[i] = false;
                }
            }

            for (int i = 0; i < symbolChangeStatus_.size(); ++i) {

                if (symbolChangeStatus_[i]) {
                    MarketChangeEvent event(brokerId_, i, symbolMarketStates_[i].get());
                    notifyMarketChanges(event);
                }
            }

        } else if (type == FixMessageType::SNAPSHOT) {

            const auto &snapshotMsg = static_cast<const FixSnapshotMessage&>(msg);
            const auto& entries = snapshotMsg.getSnapshotEntries();
            auto symbolId = snapshotMsg.getSymbolID();
            symbolMarketStates_[symbolId]->addSnapshot(entries);

        }
    } else if (event.getEventType() == EventType::LOGON_SUCCESS) {
        FixMarketDataRequest marketDataRequest(interestedSymbols_.size());

        for (auto symbol : interestedSymbols_) {
            marketDataRequest.addSymbol(symbol);
        }

        marketDataRequest.setRequestID(++mdReqID_);
        marketDataRequest.addEntryType(EntryType::Types::BID);
        marketDataRequest.addEntryType(EntryType::Types::OFFER);
        marketDataRequest.addEntryType(EntryType::Types::TRADE);
        marketDataRequest.setSubscriptionType(SubscriptionRequestType::Types::SNAPSHOT_AND_UPDATE);
        marketDataRequest.setUpdateType(UpdateType::Types::INCREMENTAL_REFRESH);
        
        if (!this->addMessageToSend(marketDataRequest)) {
            MarketDataRequestSendingFailed log(this->getSessionID());
            this->getLogger().logMessage(log);
            return;
        }
    }
}

template<typename MsgParser, typename MsgBuilder, typename Logger> void FeedHandler<MsgParser, MsgBuilder, Logger>::notifyMarketChanges(const MarketChangeEvent& event)
{
    for (auto& subscriber : marketUpdateSubscribers_) {
        subscriber.notify(event);
    }
}

#endif