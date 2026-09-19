#ifndef TYPE_DEF_H
#define TYPE_DEF_H

#include <cstdint>
#include <iostream>
#include "Decimal.h"

constexpr unsigned int PriceNumDecimals = 4;
using Price = Decimal<long long, PriceNumDecimals>;
using Volume = int;
using TimeStamp = unsigned long long;
using SymbolID = int;
using BrokerID = int;
using RptSeqType = int;
using SessionIDType = uint;
using PortType = int;

constexpr SymbolID NoSymbolID = -1;
constexpr BrokerID NoBrokerID = -1;
constexpr uint HostNameMaxSize = 100;

enum class OrderSide {
    BUY,
    SELL,
    UNKNOWN
};

#endif