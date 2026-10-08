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


enum class ResetSeqNumFlag {
    NO,
    YES,
    UNKNOWN
};


enum class SubscriptionRequestType {
    SNAPSHOT,
    SNAPSHOT_AND_UPDATE,
    CANCEL,
    UNKNOWN
};

enum class EntryType  {
    BID,
    OFFER,
    TRADE,
    UNKNOWN
};

enum class UpdateAction {
    NEW,
    CHANGE,
    DELETE,
    UNKNOWN
};

enum class AggressorIndicator {
    PASSIVE,
    AGGRESSOR,
    UNKNOWN
};


enum class UpdateType {
    FULL_REFRESH = 0,
    INCREMENTAL_REFRESH,
    UNKNOWN
};

enum class MessageType {
    HEART_BEAT,
    LOGON,
    LOGOUT,
    MARKET_DATA_REUEST,
    MARKET_DATA,
    SNAPSHOT,
    HEADER,
    UNDEFINED
};

enum class MessageProtocol {
   FIX42 = 0,
   FIX44,
   UNKNOWN
};

#endif