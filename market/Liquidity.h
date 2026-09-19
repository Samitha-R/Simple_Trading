#ifndef LIQUIDITY_H
#define LIQUIDITY_H

#include <cstdint>
#include <vector>
#include "TypeDef.h"
#include "CommonUtils.h"

using PriceVolumePair = std::pair<Price,Volume>;
constexpr Price NoPrice = -1;
constexpr Volume NoVolume = -1;
constexpr int NoIndex = -1;

struct LiquidityInfo {
    Volume volume_ = 0;

    operator bool() const{
        return volume_ > 0;
    }
};

enum class LiquidityUpdateStatus { UNKNOWN, NO_SNASHOT, OLD_UPDATE, MISSING_UPDATE, PRICE_OUT_OF_RANGE, SUCCESSFULL};

class Liquidity
{
public:

    class Iterator
    {
    public:
        Iterator(Liquidity& liquidity, int currentIndex);
        void operator+=(int distance);
        void operator-=(int distance);
        bool operator==(const Iterator& cmp) const;
        bool operator!=(const Iterator& cmp) const;
        LiquidityInfo& operator*();
    private:
        Liquidity& liquidity_;
        int currentIndex_;
    };
    class ReverseIterator
    {
    public:
        ReverseIterator(Liquidity& liquidity, int currentIndex);
        void operator+=(int distance);
        void operator-=(int distance);
        bool operator==(const ReverseIterator& cmp) const;
        bool operator!=(const ReverseIterator& cmp) const;
        LiquidityInfo& operator*();
    private:
        Liquidity& liquidity_;
        int currentIndex_;
    };
protected:
    Liquidity(std::size_t size, int tickSize);
public:
    PriceVolumePair getBestLiquidity() const;
public:
    void clear();
    std::size_t getSize() const { return size_; };
    int getTickSize() const { return tickSize_; }
    bool isBestChangeWithLastUpdate() const { return topChanges_; }
    Iterator begin();
    Iterator end();
    ReverseIterator rbegin();
    ReverseIterator rend();
    const LiquidityInfo& operator[](int index) const { return volumes_[(index + startIndex_) & mask_]; }
    LiquidityInfo& operator[](int index) { return volumes_[(index + startIndex_) & mask_]; }
protected:
    inline Price getPriceFromIndex(int index) const;
    inline int getIndexFromPrice(Price price) const;
    inline int getOffsetFromStartIndex(Price price) const;
protected:
    std::size_t size_;
    int mask_ = 0;
    int midOffset_ = 0;
    int tickSize_;
    Price startIndexPrice_ = NoPrice;
    int startIndex_ = 0;
    int endIndex_ = 0;
    std::vector<LiquidityInfo> volumes_;
    Price bestPrice_ = NoPrice;
    Volume bestVolume_ = NoVolume;
    bool topChanges_ = false;
};

inline Price Liquidity::getPriceFromIndex(int index) const
{
    int offset = index - startIndex_;

    if (offset < 0)
        offset += size_;

    return startIndexPrice_ + offset * tickSize_;     
}

inline int Liquidity::getIndexFromPrice(Price price) const
{ 
    return (getOffsetFromStartIndex(price) + startIndex_) & mask_;
}

inline int Liquidity::getOffsetFromStartIndex(Price price) const
{
    return  (price.rawValue() - startIndexPrice_.rawValue())/ tickSize_ ;
}

class BidLiquidity : public Liquidity
{
friend class BidLiquidityIterator;
public:
    BidLiquidity(std::size_t size, int tickSize);
    LiquidityUpdateStatus update(Price price, const LiquidityInfo& info);
    void copyTo(BidLiquidity &liquidity) const;
private:
    void shiftTowardsHigherPrices(int distance,  Price price, const LiquidityInfo& info);
    bool shiftTowardsLowerPrices(int distance,  Price price, const LiquidityInfo& info);
};


class AskLiquidity : public Liquidity
{
friend class AskLiquidityIterator;
public:
    AskLiquidity(std::size_t size, int tickSize);
    LiquidityUpdateStatus update(Price price, const LiquidityInfo& info);
    void copyTo(AskLiquidity &liquidity) const;
private:
    bool shiftTowardsHigherPrices(int distance, Price price, const LiquidityInfo& info);
    void shiftTowardsLowerPrices(int distance, Price price, const LiquidityInfo& info);
};

#endif
