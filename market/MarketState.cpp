#include "MarketState.h"


SymbolMarketState::SymbolMarketState(const SymbolMarketStateConfig& config, BrokerID brokerID, SymbolID symbolID): brokerID_(brokerID), symbolID_(symbolID), 
    tmpBuffer_(config.getTradeSize()), bidLiquidity_(config.getL2BookSize(), config.getL2BookTickSize()), askLiquidity_(config.getL2BookSize(), config.getL2BookTickSize())
{

}

void SymbolMarketState::clear()
{
    askLiquidity_.clear();
    bidLiquidity_.clear();
}

LiquidityUpdateStatus  SymbolMarketState::addSnapshot(const std::vector<MarketUpdate> &marketdata)
{

    LiquidityUpdateStatus status = LiquidityUpdateStatus::UNKNOWN;

    for (auto &data : marketdata) {
        auto statusL = updateBook(data);

        if (status != LiquidityUpdateStatus::SUCCESSFULL) {
            status = statusL;
        }

    }
    
    if (status == LiquidityUpdateStatus::SUCCESSFULL) {
        lastRptSeq_ = marketdata[0].getRptSeq();
        currentStatus_ = Status::SNASHOT_ADDED;
    }

    if (tmpBuffer_.size()) {
        addUpdate(tmpBuffer_);
        tmpBuffer_.clear();
    }

    return status;
}

LiquidityUpdateStatus  SymbolMarketState::addUpdate(const std::vector<MarketUpdate> &marketdata)
{
    LiquidityUpdateStatus status = LiquidityUpdateStatus::UNKNOWN;

    if (currentStatus_ != Status::SNASHOT_ADDED) {

        for (auto &data : marketdata) {
            tmpBuffer_.push_back(data);
        }
        return LiquidityUpdateStatus::NO_SNASHOT;
    }

    RptSeqType newRptSeq = lastRptSeq_, newTradeRptSeq = lastTradeRptSeq_;

    bool pendingUpdates = false;

    auto sendBookUpdates = [this, &pendingUpdates] () {
        BookUpdateEventL10 event(brokerID_, symbolID_);
        askLiquidity_.getSnapshot(event.getAskLiquidity());
        bidLiquidity_.getSnapshot(event.getBidLiquidity());
        event.setUpdatedTime(lastUpdatedTime_);
        notifyMarketChanges(event);
        pendingUpdates = false;
    };

    for (auto &data : marketdata) {

        if (data.getEntryType() == EntryType::BID || data.getEntryType() == EntryType::OFFER ) {

            if (data.getRptSeq() < lastRptSeq_) {
                continue;
            } else if (data.getRptSeq() > lastRptSeq_ + 1) {
                status = LiquidityUpdateStatus ::MISSING_UPDATE;
                pendingUpdates = false;
                break;
            }

            lastRptSeq_ = data.getRptSeq();

            auto statusL = updateBook(data);

            if (status != LiquidityUpdateStatus::SUCCESSFULL) {
                status = statusL;
                pendingUpdates = true;
            }


        } else {

            sendBookUpdates();

            if (data.getRptSeq() < lastTradeRptSeq_) {
                continue;
            } 

            TradeEvent trade(brokerID_, data);
            notifyMarketChanges(trade);

            status = LiquidityUpdateStatus::SUCCESSFULL;
            lastTradeRptSeq_ = data.getRptSeq();
        }
    }
    
    if (status == LiquidityUpdateStatus ::MISSING_UPDATE) {
        currentStatus_ = Status::INVALID;
    } else {
        sendBookUpdates();
        currentStatus_ = Status::VALID;
    }

    return status;
}


LiquidityUpdateStatus SymbolMarketState::updateBook(const MarketUpdate &data)
{
    LiquidityUpdateStatus updateSuccess;

    if (data.getEntryType() == EntryType::OFFER)  {
        updateSuccess = askLiquidity_.update(data.getPrice(), {data.getVolume()});
    } else {
        updateSuccess = bidLiquidity_.update(data.getPrice(), {data.getVolume()});
    }
    
    lastUpdatedTime_ = data.getTimeStamp();

    return updateSuccess;

}

void SymbolMarketState::notifyMarketChanges(const EventBase& event)
{
    for (auto subscriber : marketUpdateSubscribers_) {
        subscriber.notify(event);
    }
}

void SymbolMarketState::sendBookUpdate() {
    BookUpdateEventL10 event;
    askLiquidity_.getSnapshot(event.getAskLiquidity());
    bidLiquidity_.getSnapshot(event.getBidLiquidity());
    event.setUpdatedTime(lastUpdatedTime_);
    notifyMarketChanges(event);
}
