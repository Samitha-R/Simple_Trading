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

struct LiquidityInfo 
{
    LiquidityInfo() = default;
    LiquidityInfo(Volume volume) : volume_(volume) {}

    explicit operator bool() const { return volume_ > 0;}

    auto operator<=>(const LiquidityInfo& c) const =  default;

    Volume volume_ = 0;
};

inline std::ostream& operator<<(std::ostream& os, const LiquidityInfo& liquidityInfo) 
{
    return os << liquidityInfo.volume_;
}

enum class LiquidityUpdateStatus { UNKNOWN, NO_SNASHOT, OLD_UPDATE, MISSING_UPDATE, PRICE_OUT_OF_RANGE, SUCCESSFULL};
enum class LiquidityType { BID = -1, ASK = 1};

template<unsigned int size, LiquidityType type>  class LiquiditySnapshot
{
    template<LiquidityType typeF>  friend class Liquidity;
public:
    LiquiditySnapshot(Price startIndexPrice, int tickSize) : startIndexPrice_(startIndexPrice), tickSize_(tickSize) {}
    LiquiditySnapshot() {}
public:
    void setStartIndexPrice(Price startIndexPrice) { startIndexPrice_ = startIndexPrice; }
    void setTickSize(int tickSize) {int tickSize_ = tickSize; }
    std::size_t getSize() const { return size; };
    int getTickSize() const { return tickSize_; }
    const LiquidityInfo& operator[](unsigned int index) const { return volumes_[index ]; }
    LiquidityInfo& operator[](unsigned int index) { return volumes_[index]; }
    Price getPriceAt(unsigned int index) const { return startIndexPrice_ + Price(index * tickSize_ * static_cast<int>(type)); }
private:
    Price startIndexPrice_ = NoPrice;
    int tickSize_ = 1;
    std::array<LiquidityInfo, size> volumes_;
};

template<LiquidityType type> class Liquidity
{
public:
    Liquidity(unsigned int size, int tickSize);
public:
    PriceVolumePair getBestLiquidity() const;
public:
    std::size_t getSize() const { return size_; };
    int getTickSize() const { return tickSize_; }
    const LiquidityInfo& operator[](int offset) const { return volumes_[(offset + startIndex_) & mask_]; }
    LiquidityInfo& operator[](int offset) { return volumes_[(offset + startIndex_) & mask_]; }
    Price getPriceFromOffset(int offset) const { return startIndexPrice_ + Price(offset * tickSize_ * static_cast<int>(type)); }
    LiquidityUpdateStatus update(Price price, const LiquidityInfo& info);
    template<unsigned int size> void getSnapshot(LiquiditySnapshot<size,type>& snapshot) const;
    void clear();
private:
    Price getPriceFromIndex(int index) const;
    int getIndexFromPrice(Price price) const;
    int getOffsetFromStartIndex(Price price) const;
    void shiftTowardsBestPrice(int distance, Price price, const LiquidityInfo& info);
    bool shiftOppositBestPrice(int distance, Price price, const LiquidityInfo& info);
protected:
    int size_ = 0;
    int mask_ = 0;
    int midOffset_ = 0;
    int tickSize_;
    Price startIndexPrice_ = NoPrice;
    int startIndex_ = 0;
    int endIndex_ = 0;
    std::vector<LiquidityInfo> volumes_;
    Price bestPrice_ = NoPrice;
    Volume bestVolume_ = NoVolume;
};

template<LiquidityType type> Liquidity<type>::Liquidity(unsigned int size, int tickSize): 
    size_(size), mask_(size - 1), tickSize_(tickSize), endIndex_(size > 0 ? size - 1 : 0), midOffset_(size / 2), volumes_(size)
{
    if (!isPowerOfTwo(size_) || size_ == 0) {
        throw std::invalid_argument("size of Liquidity must be a power of two ");
    }

    if (!isPowerOfTen(tickSize)) {
        throw std::invalid_argument("tickSize of Liquidity must be a power of ten ");
    }
}

template<LiquidityType type>  Price Liquidity<type>::getPriceFromIndex(int index) const
{
    int offset = ((index - startIndex_) * static_cast<int>(type)) & mask_;
    return startIndexPrice_ + Price(offset * tickSize_ );     
}

template<LiquidityType type> int Liquidity<type>::getIndexFromPrice(Price price) const
{ 
    return (getOffsetFromStartIndex(price) + startIndex_) & mask_;
}

template<LiquidityType type> int Liquidity<type>::getOffsetFromStartIndex(Price price) const
{
    return  ((price.rawValue() - startIndexPrice_.rawValue()) * static_cast<int>(type))/ tickSize_ ;
}

template<LiquidityType type> PriceVolumePair Liquidity<type>::getBestLiquidity() const
{
    return std::pair(bestPrice_,bestVolume_);
}

template<LiquidityType type> LiquidityUpdateStatus Liquidity<type>::update(Price price, const LiquidityInfo& info)
{

    if (startIndexPrice_ == NoPrice) {
        startIndexPrice_ = price - Price(midOffset_ * tickSize_ * static_cast<int>(type));
    }

    int distance = getOffsetFromStartIndex(price);
    bool volumeAdded = true;

    if (distance >= 0 && distance < size_) {
        int index =  (distance + startIndex_) & mask_;
        volumes_[index] = info;
    } else if (distance < 0) {
        shiftTowardsBestPrice(distance, price, info);
    } else  {
        
        if (!shiftOppositBestPrice(distance, price, info))
            return LiquidityUpdateStatus::PRICE_OUT_OF_RANGE;
    }

    if (info) {

        if (bestPrice_ == NoPrice || bestPrice_ == price ) {
            bestPrice_ = price;
            bestVolume_ = info.volume_;

        } else {
            if constexpr (type == LiquidityType::BID) {

                if (bestPrice_ < price) {
                    bestPrice_ = price;
                    bestVolume_ = info.volume_;
                }

            } else {

                if (bestPrice_ >= price) {
                    bestPrice_ = price;
                    bestVolume_ = info.volume_; 
                }
            }
        }

    } else if (price == bestPrice_) {
        bestVolume_ = NoVolume;
        bestPrice_ = NoPrice;
        int topValIndex = getIndexFromPrice(price);

        for (int i = topValIndex + 1; i != startIndex_ ;) {

            if (volumes_[i] != 0) {
                bestPrice_ = getPriceFromIndex(i);
                bestVolume_ = volumes_[i].volume_;
                break;
            }
            i = (i + 1) & mask_;
        }
    }

    return LiquidityUpdateStatus::SUCCESSFULL;
}

template<LiquidityType type> bool Liquidity<type>::shiftOppositBestPrice(int distance,  Price price, const LiquidityInfo& info)
{
    auto numShiftReq = std::min(distance - static_cast<int>(size_) + 1, static_cast<int>(size_));

    int numShift = 0;

    for (int i = startIndex_; numShift < numShiftReq; ++numShift) {
        
        if (volumes_[i] != 0)
            return false;

        i = (i + 1) & mask_;
    }

    if (numShift < size_) {
        startIndex_ = (startIndex_ + numShiftReq) & mask_;
        endIndex_ = (endIndex_ + numShiftReq) & mask_;
        volumes_[endIndex_] = info;
        startIndexPrice_ = startIndexPrice_ + Price(tickSize_ * numShift * static_cast<int>(type));
    } else {
        volumes_[(startIndex_ + midOffset_) & mask_] = info;
        startIndexPrice_ = price + Price(midOffset_ * tickSize_ * static_cast<int>(type));
    }

    return true;
}

template<LiquidityType type> void Liquidity<type>::shiftTowardsBestPrice(int distance,  Price price, const LiquidityInfo& info)
{
    int numShiftReq = -distance;
    int totalShifts = 0;

    if (numShiftReq < size_) {
        for (int numShift = 1; numShift <= numShiftReq; ++numShift) {
            volumes_[(startIndex_ - numShift) & mask_] = LiquidityInfo();
        }

        startIndex_ = (startIndex_ - numShiftReq) & mask_;
        endIndex_ = (endIndex_ - numShiftReq) & mask_;
        startIndexPrice_ = startIndexPrice_ - Price(tickSize_ * numShiftReq * static_cast<int>(type));
        volumes_[startIndex_] = info; 
    } else {
        std::fill(volumes_.begin(), volumes_.end(), LiquidityInfo());
        bestPrice_ = NoPrice;
        bestVolume_ = NoVolume;
        volumes_[(startIndex_ + midOffset_) & mask_] = info;
        startIndexPrice_ = price - Price(midOffset_ * tickSize_ * static_cast<int>(type));
    }
}

template<LiquidityType type> template<unsigned int size> void Liquidity<type>::getSnapshot(LiquiditySnapshot<size,type>& snapshot) const
{
    if (bestPrice_ == NoPrice)
        return;

    auto topIndex   = getIndexFromPrice(bestPrice_);
    snapshot.startIndexPrice_ = bestPrice_;
    snapshot.tickSize_ = tickSize_;

    for (int i = topIndex, j = 0; ; i = (i + 1) & mask_, ++j) {
        snapshot[j] = volumes_[i];

        if (i == endIndex_ || j == size - 1)
            break;
    }
}

template<LiquidityType type> void Liquidity<type>::clear()
{
    startIndexPrice_ = NoPrice;
    std::fill(volumes_.begin(), volumes_.end(), LiquidityInfo());
    Price bestPrice_ = NoPrice;
    Volume bestVolume_ = NoVolume;
    bool topChanges_ = false;  
}

template<LiquidityType type> std::ostream& operator<<(std::ostream& os, const Liquidity<type>& l) 
{
     
    auto size = l.getSize();

    for (int i = 0; i < size; ++i) {
        os << l.getPriceFromOffset(i) << ":" << l[i] << " ";
    }
    return os;
}

template<auto size, LiquidityType type> std::ostream& operator<<(std::ostream& os, const LiquiditySnapshot<size,type>& l) 
{

    for (int i = 0; i < size; ++i) {
        os << l.getPriceAt(i) << ":" << l[i] << " ";
    }
    return os;
}

#endif
