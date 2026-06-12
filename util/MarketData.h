#ifndef MARKET_DATA_H
#define MARKET_DATA_H

#include <cstdint>
#include <iostream>

using Price = double;
using Volume = int;
using TimeStamp = unsigned long long;
using SymbolID = int;
using BrokerID = int;
constexpr SymbolID NoSymbolID = -1;
constexpr BrokerID NoBrokerID = -1;

class Marketdata {
friend inline std::ostream& operator<<(std::ostream& os, const Marketdata& mt);
public:
    Marketdata(TimeStamp timeStamp, Price bidPrice, Price askPrice, SymbolID symbol, Volume bidVolume, Volume askVolume) :
        timeStamp_(timeStamp), bidPrice_(bidPrice), askPrice_(askPrice), symbol_(symbol), bidVolume_(bidVolume), askVolume_(askVolume) {}
    Marketdata() = default;
public:
    TimeStamp getTimeStamp() const { return timeStamp_; }
    Price getBidPrice() const { return bidPrice_; }
    Price getAskPrice() const { return askPrice_; }
    SymbolID getSymbolID() const { return symbol_; }
    Volume getBidVolume() const { return bidVolume_;}
    Volume getAskVolume() const { return askVolume_; }

private:
    TimeStamp timeStamp_ = 0;
    Price bidPrice_ = 0;
    Price askPrice_ = 0;
    SymbolID symbol_ = 0;
    Volume bidVolume_ = 0;
    Volume askVolume_ = 0;
};

inline std::ostream& operator<<(std::ostream& os, const Marketdata& mt) {
    os << mt.timeStamp_ << " " << mt.symbol_ << " " << mt.bidPrice_ << " " << mt.askPrice_ << " " << mt.bidVolume_ << " " << mt.askVolume_;
    return os;
}

inline int getIndex(const Marketdata &data) {
    return data.getSymbolID();
}

class Trade
{
public:
    enum class Side { Sell, Buy , NotDefined};
public:
    Trade(SymbolID symbol, TimeStamp timeStamp, Price price, Volume volume, Side side = Side::NotDefined, uint64_t id = 0) :
        symbol_(symbol), timeStamp_(timeStamp), price_(price), volume_(volume), side_(side), id_(id) {}
    Trade() = default;
    SymbolID getSymbolID() const { return symbol_; }
    TimeStamp getTimeStamp()  const { return timeStamp_; }
    Price getPrice() const { return price_; }
    Volume getVolume() const { return volume_; }
    Side getSide() const { return side_; }
    uint64_t getId() const { return id_; }
private:
    SymbolID symbol_ = 0;
    TimeStamp timeStamp_ = 0;
    Price price_ = 0;
    Volume volume_ = 0;
    Side side_ = Side::NotDefined;
    uint64_t id_ = 0;
};

#endif