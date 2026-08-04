#include "L2Book.h"


L2Book::L2Book(std::size_t size, double tickSize) : bidLiquidity_(size, tickSize), askLiquidity_(size, tickSize)
{

}

L2Book::L2Book() : L2Book(2048, 0.01) {}

void L2Book::clear()
{
    askLiquidity_.clear();
    bidLiquidity_.clear();
}

LiquidityUpdateStatus L2Book::update(const FixMarketUpdate &data) {

    LiquidityUpdateStatus updateSuccess;

    if (data.getEntryType() == EntryType::Types::OFFER)  {
        updateSuccess = askLiquidity_.update(data.getPrice(), {data.getVolume()});
    } else {
        updateSuccess = bidLiquidity_.update(data.getPrice(), {data.getVolume()});
    }
    
    lastUpdatedTime_ = data.getTimeStamp();

    return updateSuccess;
}

void L2Book::copyTo(L2Book &l2Book) const
{
    bidLiquidity_.copyTo(l2Book.bidLiquidity_);
    askLiquidity_.copyTo(l2Book.askLiquidity_);
    l2Book.lastUpdatedTime_ = lastUpdatedTime_;
}
