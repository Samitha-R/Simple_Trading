#ifndef MEAN_REVERSION
#define MEAN_REVERSION

#include "Strategy.h"
#include "FastRingBuffer.h"

struct BestLiquidityData
{  
    Volume volume_ = 0;
    double normalizeDiff_ = NoVolume;
};

struct TradeData
{
    Price price_ = 0;
    Volume volume_ = 0;
    TimeStamp stamp_ = 0;
};

struct MicroIncidents
{
    BestLiquidityData bid_;
    BestLiquidityData ask_;
    Price midPrice_ = 0;
    double imbalance_ = -1;
    Price spread_ = 0;
    TimeStamp time_ = 0;
};

struct IncidentStatus
{
    enum Status { Undefined = 0, Stable = 1, Increasing =  2, Decreasing = 4 };
};

class MeanReversion : public  StrategyBase
{
public:
    MeanReversion(std::size_t eventQueueSize, BrokerID brokerId, SymbolID symbol, std::size_t microStructureWindowSize, std::size_t tradeWindowSize, TimeStamp microStructureTime, TimeStamp analysisTime);
    void setVolumeStableThreshold(double threshold) { volumeStableThreshold_ = threshold; }
    double getVolumeStableThreshold() { return volumeStableThreshold_; }
    void setTradeStableThreshold(double threshold) { tradeStableThreshold_ = threshold; }
    double getTradeStableThreshold() { return tradeStableThreshold_; }
    void handleEvents();
private:
    void handleEvent(const BookUpdateEventL10& event);
    void handleEvent(const TradeEvent& event);
    void handleEvent(const SymbolStatusEvent& event);
    void handleEvent(const BrokerStatusEvent& event);
    void doAnalysys();
    int getMidPriceStatus();
    int getSpreadStatus();
    std::tuple<int,int, double> getLiquidityStatus(bool bid);
    int getTradeStatus(FastRingBuffer<TradeData> &trades);
private:
    SymbolID symbol_;
    BrokerID brokerId_;
    std::size_t microStructureWindowSize_ = 0;
    std::size_t tradeWindowSize_ = 0;
    TimeStamp microStructureTime_ = 0;
    TimeStamp analysisTime_ = 0;
    TimeStamp latestTime_ = 0;
    double volumeStableThreshold_ = 0.2;
    double tradeStableThreshold_ = 0.2;
    FastRingBuffer<MicroIncidents> incidentWindow_;
    FastRingBuffer<TradeData> buyTradesWindow_;
    FastRingBuffer<TradeData> sellTradesWindow_;
    FastRingBuffer<BookUpdateEventL10> bookSnapshots_;
    Volume maxAllowedShares_ = 0;
    Volume ownShares_ = 0;
    Price avgBuyPrice_ = 0;
    Price bestBidPrice_ = 0;
    std::array<Volume,2> pendingShares_;
    std::array<Volume,2> pendingCancellations_;
    Price availbleBalance = 0;
    OrderID currnetORderID_ = 1;
};

#endif