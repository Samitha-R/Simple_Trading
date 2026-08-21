#ifndef TYPE_DEF_H
#define TYPE_DEF_H

#include <cstdint>
#include <iostream>

using Price = double;
using Volume = int;
using TimeStamp = unsigned long long;
using SymbolID = int;
using BrokerID = int;
using RptSeqType = int;

constexpr SymbolID NoSymbolID = -1;
constexpr BrokerID NoBrokerID = -1;

#endif