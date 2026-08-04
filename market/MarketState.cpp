#include "MarketState.h"


SymbolMarketState::SymbolMarketState(const SymbolMarketStateConfig& config):
    tradeSize_(config.getTradeSize()), trades_(0), l2Book_(config.getL2BookSize(), config.getL2BookTickSize())
{
    tradeSeqNo_.store(0, std::memory_order_release);
    updateCount_.store(0, std::memory_order_release);
}

void SymbolMarketState::clear()
{
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    l2Book_.clear();
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
}

LiquidityUpdateStatus  SymbolMarketState::addSnapshot(const std::vector<FixMarketUpdate> &marketdata)
{

    LiquidityUpdateStatus status = LiquidityUpdateStatus::UNKNOWN;
    updateCount_.fetch_add(1, std::memory_order_acq_rel);

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

    updateCount_.fetch_add(1, std::memory_order_acq_rel);

    if (tmpBuffer_.size()) {
        addUpdate(tmpBuffer_);
        tmpBuffer_.clear();
    }

    return status;
}

LiquidityUpdateStatus  SymbolMarketState::addUpdate(const std::vector<FixMarketUpdate> &marketdata)
{
    LiquidityUpdateStatus status = LiquidityUpdateStatus::UNKNOWN;

    if (currentStatus_ != Status::SNASHOT_ADDED) {

        for (auto &data : marketdata) {
            tmpBuffer_.push_back(data);
        }
        return LiquidityUpdateStatus::NO_SNASHOT;
    }

    RptSeqType newRptSeq = lastRptSeq_, newTradeRptSeq = lastTradeRptSeq_;

    updateCount_.fetch_add(1, std::memory_order_acq_rel);

    for (auto &data : marketdata) {

        if (data.getEntryType() == EntryType::Types::BID || data.getEntryType() == EntryType::Types::OFFER ) {

            if (data.getRptSeq() < lastRptSeq_) {
                continue;
            } else if (data.getRptSeq() > lastRptSeq_ + 1) {
                status = LiquidityUpdateStatus ::MISSING_UPDATE;
                break;
            }

            lastRptSeq_ = data.getRptSeq();

            auto statusL = updateBook(data);

            if (status != LiquidityUpdateStatus::SUCCESSFULL) {
                status = statusL;
            }


        } else {

            if (data.getRptSeq() < lastTradeRptSeq_) {
                continue;
            } 
            updateTrade(data);
            status = LiquidityUpdateStatus::SUCCESSFULL;
            lastTradeRptSeq_ = data.getRptSeq();
        }
    }
    
    if (status == LiquidityUpdateStatus ::MISSING_UPDATE)
        currentStatus_ = Status::INVALID;
    else
        currentStatus_ = Status::VALID;

    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    return status;

}


LiquidityUpdateStatus SymbolMarketState::updateBook(const FixMarketUpdate &data)
{
    return l2Book_.update(data);
    //auto topAsk = l2Book_.getMinAskPriceVolume();
    //l1Book_.setBestAskPrice(topAsk.first);
    //l1Book_.setBestAskVolume(topAsk.second);

    //if (l1Book_.getTime() < data.getTimeStamp())
      //  l1Book_.setUpdatedTime(data.getTimeStamp());

}

void SymbolMarketState::updateTrade(const FixMarketUpdate &data)
{
    auto seqNo = tradeSeqNo_.load(std::memory_order_acquire);
    auto indexNo = seqNo % tradeSize_;
    trades_[indexNo] = data;
    tradeSeqNo_.fetch_add(1, std::memory_order_acq_rel);
   // l1Book_.setLastTrade(data);

   // if (l1Book_.getTime() < data.getTimeStamp())
     //   l1Book_.setUpdatedTime(data.getTimeStamp());
}
