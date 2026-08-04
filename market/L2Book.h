#ifndef L2_BOOK_MAP_H
#define L2_BOOK_MAP_H

#include <vector>
#include <atomic>
#include "Liquidity.h"
#include "FixMessage.h"

class L2Book
{
public:
    L2Book();
    L2Book(std::size_t size, double tickSize);
    void clear();
    LiquidityUpdateStatus update(const FixMarketUpdate &data);
    bool isBestBidChangeWithLastUpdate() const { return bidLiquidity_.isBestChangeWithLastUpdate(); }
    bool isBestAskChangeWithLastUpdate() const { return askLiquidity_.isBestChangeWithLastUpdate(); }
    PriceVolumePair getMaxBidPriceVolume() const { return bidLiquidity_.getBestLiquidity(); }
    PriceVolumePair getMinAskPriceVolume() const { return askLiquidity_.getBestLiquidity(); }
    BidLiquidity& getBidLiquidity() { return bidLiquidity_; }
    AskLiquidity& getAskLiquidity() { return askLiquidity_; }
    const BidLiquidity& getBidLiquidity() const { return bidLiquidity_; }
    const AskLiquidity& getAskLiquidity() const { return askLiquidity_; }
    void copyTo(L2Book &l2Book) const;
    TimeStamp getTimeStamp() const { return lastUpdatedTime_; }
private:
    BidLiquidity bidLiquidity_;
    AskLiquidity askLiquidity_;
    TimeStamp lastUpdatedTime_;
};
#endif