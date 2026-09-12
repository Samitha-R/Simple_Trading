#include "MeanReversion.h"
#include <optional>

MeanReversion::MeanReversion(std::size_t eventQueueSize, SymbolID symbol, std::size_t microStructureWindowSize, std::size_t tradeWindowSize, TimeStamp microStructureTime, TimeStamp analysisTime) :
                StrategyBase(eventQueueSize),
                symbol_(symbol), microStructureWindowSize_(microStructureWindowSize), tradeWindowSize_(tradeWindowSize), analysisTime_(analysisTime),
                microStructureTime_(microStructureTime), incidentWindow_(microStructureWindowSize),
                buyTradesWindow_(microStructureWindowSize), sellTradesWindow_(tradeWindowSize),
                l2SnapShots_(2, L2Book(16, 0.01))
{

}

void MeanReversion::handleEvents()
{
    auto slot = events_.getReadSlot();

    while (slot) {
        
        auto data = slot->getData();
        events_.setReadComplete(slot);

        if (std::holds_alternative<MarketChangeEvent>(data)) {
            handleEvent(std::get<MarketChangeEvent>(data));
        }
    }
}

void MeanReversion::handleEvent(const MarketChangeEvent& event)
{
    auto &l2snap = l2SnapShots_[(snapShotUsage_++) % 2];
    auto& symbolMarketState = event.getMarketState();
    std::size_t updateCount1, updateCount2;
    TimeStamp lastUpdateTime = 0;

    do {
        updateCount1 = symbolMarketState.getUpdateCount();
        auto l2book = symbolMarketState.getL2Book();
        l2book.copyTo(l2snap);

        auto tradeSeqNum = symbolMarketState.getTradeSeuence();
        const auto& trades = symbolMarketState.getTrades();
        auto bestBid = l2book.getBidLiquidity().getBestLiquidity();
        lastUpdateTime = l2book.getTimeStamp();

        while (nextTradeSeq_ !=  tradeSeqNum) {
            auto  &t = symbolMarketState.getTrade(nextTradeSeq_);
            auto price = t.getPrice();

            if (price <= bestBid.first) {
                sellTradesWindow_.push(TradeData{t.getPrice(), t.getVolume(), t.getTimeStamp()});
            } else {
                buyTradesWindow_.push(TradeData{t.getPrice(), t.getVolume(), t.getTimeStamp()});
            }

            ++nextTradeSeq_;
            
            if (lastUpdateTime < t.getTimeStamp())
                lastUpdateTime = t.getTimeStamp();
        }

        updateCount2 = symbolMarketState.getUpdateCount();


   } while ((updateCount1 != updateCount2) || (updateCount2 % 2));

    latestTime_ = latestTime_ < lastUpdateTime ? lastUpdateTime : latestTime_;

    auto &bidLiquidity_ = l2snap.getBidLiquidity();
    auto bestBidLiquidity = bidLiquidity_.getBestLiquidity();

    Volume bestBidVolumes = 0;

    {
        auto iterB = bidLiquidity_.begin();
        auto iterE = bidLiquidity_.end();

        for ( ; iterB != iterE; iterB+=1) {
            bestBidVolumes += *iterB;
        }
    }

    
    auto &askLiquidity_ = l2snap.getAskLiquidity();
    auto bestAskLiquidity = askLiquidity_.getBestLiquidity();

    Volume bestAskVolumes = 0;

    {
        auto iterB = askLiquidity_.begin();
        auto iterE = askLiquidity_.end();

        for ( ; iterB != iterE; iterB+=1) {
            bestAskVolumes += *iterB;
        }
    }

    MicroIncidents incident;
    incident.bid_.volume_ = bestBidVolumes;
    incident.ask_.volume_ = bestAskVolumes;

    if (incidentWindow_.count()) {
        auto &prevIncident = incidentWindow_.back();
        prevIncident.bid_.normalizeDiff_ = static_cast<double>(incident.bid_.volume_ - prevIncident.bid_.volume_) / prevIncident.bid_.volume_;
        prevIncident.ask_.normalizeDiff_ = static_cast<double>(incident.ask_.volume_ - prevIncident.ask_.volume_) / prevIncident.ask_.volume_;
    }

    incident.midPrice_ = (bestBidLiquidity.first + bestAskLiquidity.first) / 2;
    incident.time_ = latestTime_;
    incident.spread_ = bestAskLiquidity.first - bestAskLiquidity.first;
    incident.imbalance_ = static_cast<double>(bestBidVolumes - bestAskVolumes) / (bestAskVolumes + bestAskVolumes);
    incidentWindow_.push(incident);

    TimeStamp totwindowTime = incidentWindow_.back().time_ - incidentWindow_.front().time_;

    if (totwindowTime >= microStructureTime_ && (buyTradesWindow_.count() > 0) && (sellTradesWindow_.count() > 0)) {
        Price movingAvg = 0;
        std::size_t count = 1;
        auto end = incidentWindow_.count() - 1;

        for (std::size_t i = end; count <= incidentWindow_.count(); --i, ++count) {

            if (incidentWindow_[end].time_ - incidentWindow_[i].time_ > microStructureTime_)
                break;

            movingAvg += incidentWindow_[i].midPrice_;
        }

        movingAvg = movingAvg / count;

        auto midPriceStatus = getMidPriceStatus();
        auto spreadStatus = getSpreadStatus();
        auto [bidShortTerm, bidLongTerm, bidLongTermDiff] = getLiquidityStatus(true);
        auto [askShortTerm, askLongTerm, askLongTermDiff] = getLiquidityStatus(false);
        auto sellTradeStatus= getTradeStatus(sellTradesWindow_);
        auto buyTradeStatus = getTradeStatus(buyTradesWindow_);

        if (incident.midPrice_  < movingAvg) {

            if ((midPriceStatus == IncidentStatus::Stable) &&
                (spreadStatus == IncidentStatus::Stable) &&
                (bidShortTerm == IncidentStatus::Stable && bidLongTerm == IncidentStatus::Stable && bidLongTermDiff > 0) &&
                (askShortTerm == IncidentStatus::Stable && askLongTerm == IncidentStatus::Stable && askLongTermDiff > 0) &&
                (sellTradeStatus == IncidentStatus::Stable) &&
                (buyTradeStatus == IncidentStatus::Stable)) {
                    //make a passsive bid
                }
        }
    }
}

int MeanReversion::getMidPriceStatus()
{
    int midPriceStatus = IncidentStatus::Undefined;
    std::size_t count = 2;
    auto &lastIncident = incidentWindow_[incidentWindow_.count() - 1];

    for (std::size_t i = incidentWindow_.count() - 2; count <= incidentWindow_.count(); --i, ++count) {
        auto &incident = incidentWindow_[i];
        auto &incidentNext = incidentWindow_[i];

        if ((incident.time_ - lastIncident.time_) > analysisTime_) 
            break;

        if (incident.midPrice_ < incidentNext.midPrice_) {  // mid price is decresing
            midPriceStatus |= IncidentStatus::Increasing;
        } else if (incident.midPrice_ > incidentNext.midPrice_) {
            midPriceStatus |= IncidentStatus::Decreasing;
        } else {
            midPriceStatus |= IncidentStatus::Stable;
        }
    }

    return midPriceStatus;
}

int MeanReversion::getSpreadStatus()
{
    int spreadStatus = IncidentStatus::Undefined;
    std::size_t count = 2;
    auto &lastIncident = incidentWindow_[incidentWindow_.count() - 1];

    for (std::size_t i = incidentWindow_.count() - 2; count <= incidentWindow_.count(); --i, ++count) {
        auto &incident = incidentWindow_[i];
        auto &incidentNext = incidentWindow_[i];

        if ((incident.time_ - lastIncident.time_) > analysisTime_) 
            break;

        if (incident.spread_ < incidentNext.spread_) {  // mid price is decresing
            spreadStatus |= IncidentStatus::Increasing;
        } else if (incident.spread_ > incidentNext.spread_) {
            spreadStatus |= IncidentStatus::Decreasing;
        } else {
            spreadStatus |= IncidentStatus::Stable;
        }

    }

    return spreadStatus;
}

std::tuple<int,int, double> MeanReversion::getLiquidityStatus(bool bid)
{
    int liquidityLongTermStatus = IncidentStatus::Undefined, liquidityShortTermStatus = IncidentStatus::Undefined;
    std::size_t count = 1;
    auto &lastIncident = incidentWindow_[incidentWindow_.count() - 1];
    std::size_t i = incidentWindow_.count() - 1;

    for (; count <= incidentWindow_.count(); --i, ++count) {
        auto &incident = incidentWindow_[i];
        auto &liquidityData = bid ? incident.bid_ : incident.ask_;

        if ((incident.time_ - lastIncident.time_) > analysisTime_) 
            break;

        if (liquidityData.normalizeDiff_ > volumeStableThreshold_) {
            liquidityShortTermStatus |= IncidentStatus::Increasing;
        } else if (liquidityData.normalizeDiff_ < -volumeStableThreshold_) {
            liquidityShortTermStatus |= IncidentStatus::Decreasing;
        } else {
            liquidityShortTermStatus |= IncidentStatus::Stable;
        }
    }

    auto longTermDiff = 0;
    if (count > 1) {
        auto &firstIncident= incidentWindow_[incidentWindow_.count() - count];
        auto &firstLiquidityData = bid ? firstIncident.bid_ : firstIncident.ask_;
        auto &lastLiquidityData = bid ? lastIncident.bid_ : lastIncident.ask_;
        auto longTermDiff = static_cast<double>(lastLiquidityData.volume_ - firstLiquidityData.volume_) / (lastLiquidityData.volume_ + firstLiquidityData.volume_);

        if (longTermDiff > volumeStableThreshold_) {
            liquidityLongTermStatus |= IncidentStatus::Increasing;
        } else if (longTermDiff < -volumeStableThreshold_) {
            liquidityLongTermStatus |= IncidentStatus::Decreasing;
        } else {
            liquidityLongTermStatus |= IncidentStatus::Stable;
        }
    }

    return std::tuple(liquidityShortTermStatus, liquidityLongTermStatus, longTermDiff);
}


int MeanReversion::getTradeStatus(FastRingBuffer<TradeData> &trades)
{
    Volume tradesInAnalysisWindow = 0, tradesInWindow = 0;

    auto iterB = trades.rbegin();
    auto iterE = trades.rend();

    for ( ; iterB != iterE; iterB += 1) {
        auto& trade = *iterB;

        if (latestTime_ - trade.stamp_ <= analysisTime_) {
            tradesInAnalysisWindow += trade.volume_;
        }

        tradesInWindow  += trade.volume_;
    }

    TimeStamp sellTradeWindowTime = trades.back().stamp_ - trades.front().stamp_;
    Volume extendedAnalysisWindowSells = (tradesInAnalysisWindow / analysisTime_) * sellTradeWindowTime;
    double sellTradeDiff = static_cast<double>(extendedAnalysisWindowSells - tradesInWindow) / tradesInWindow;

    int status = IncidentStatus::Undefined;

    if (sellTradeDiff < -tradeStableThreshold_)
        status = IncidentStatus::Decreasing;
    else if (sellTradeDiff > tradeStableThreshold_)
        status = IncidentStatus::Increasing;
    else
        status = IncidentStatus::Stable;

    return status;
 }