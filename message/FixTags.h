#ifndef FIX_TAGS_H
#define FIX_TAGS_H

#include <charconv>
#include <cstdint>
#include <vector>
#include <string>
#include <cstring>
#include <array>
#include "TypeDef.h"

namespace Fix {

constexpr char SOH = '\x01';

template<std::size_t N> struct TagProperty
{
   constexpr TagProperty(const char (&tagName)[N])
   {
      for (std::size_t i = 0; i < N-1; ++i) {
         name_[i] = tagName[i];
         intValue_ = intValue_ * 10 + (tagName[i] - '0');
      }
      name_[N-1] = '\0';

   }
   std::array<char, N> name_;
   std::size_t intValue_ = 0;
};

template<TagProperty property> struct FixTag {
   static constexpr int id_ = property.intValue_;
   static constexpr auto name_ = property.name_.data();
   static constexpr std::size_t length_ = property.name_.size() - 1;
};

struct OrderType : public FixTag<"40"> {
   enum class Types : char {
      MARKET = '1',
      LIMIT = '2',
      STOP = '3',
      STOP_LIMIT = '4',
   };
};

struct Side : public FixTag<"54"> {
   enum class Types : char {
      BUY = '1',
      SELL = '2',
      SELL_SHORT = '5',
      SELL_SHORT_EXEMPT = '6',
   };
};

struct ResetSeqNumFlag : public FixTag<"141"> {
   enum class Types {
      NO = 'N',
      YES = 'Y',
   };
};

struct SubscriptionRequestType : public FixTag<"263"> {
   enum class Types : char {
      SNAPSHOT = '0',
      SNAPSHOT_AND_UPDATE = '1',
      CANCEL = '2'
   };
};

struct EntryType : public FixTag<"269"> {
   enum class Types : char {
      BID = '0',
      OFFER = '1',
      TRADE = '2'
   };
};

struct UpdateAction : public FixTag<"279"> {
   enum class Types : char {
      NEW = '0',
      CHANGE = '1',
      DELETE = '2'
   };
};

struct AggressorIndicator : public FixTag<"5797"> {
   enum class Types : char {
      PASSIVE = 'N',
      AGGRESSOR = 'Y'
   };
};

struct UpdateType : public FixTag<"265"> {
   enum class Types : char {
      FULL_REFRESH = '0',
      INCREMENTAL_REFRESH = '1'
   };
};

struct TimeInForce : public FixTag<"59"> {
   enum class Types : char {
      DAY = '0',
      GOOD_TILL_CANCEL = '1',
      AT_THE_OPENING = '2',
      IMMEDIATE_OR_CANCEL = '3',
      FILL_OR_KILL = '4',
      GOOD_TILL_CROSSING = '5',
      GOOD_TILL_DATE = '6',
   };
};

using BeginString = FixTag<"8">;
using BodyLength = FixTag<"9">;
using CheckSum = FixTag<"10">;
using ClOrdID = FixTag<"11">;
using SecuirtyIDSource = FixTag<"22">;
using LastPrice = FixTag<"31">;
using LastQuantity = FixTag<"32">;
using MsgSeqNum = FixTag<"34">;
using MsgType = FixTag<"35">;
using OrderQty = FixTag<"38">;
using PossDupFlag = FixTag<"43">;
using Price = FixTag<"44">;
using SecuirtyID = FixTag<"48">;
using SenderCompID = FixTag<"49">;
using SenderSubID = FixTag<"50">;
using SendingTime = FixTag<"52">;
using Symbol = FixTag<"55">;
using TargetCompID = FixTag<"56">;
using TargetSubID = FixTag<"57">;
using Text = FixTag<"58">;
using TransactionTime = FixTag<"60">;
using RptSeq = FixTag<"83">;
using PossResend = FixTag<"97">;
using EncryptMethod = FixTag<"98">;
using HeartBtInt = FixTag<"108">;
using TestReqID = FixTag<"112">;
using OrigSendingTime = FixTag<"122">;
using NoRelatedSym = FixTag<"146">;
using ReqID = FixTag<"262">;
using MarketDepth = FixTag<"264">;
using NoMDEntryTypes = FixTag<"267">;
using NoEntries = FixTag<"268">;
using EntryPrice = FixTag<"270">;
using EntrySize = FixTag<"271">;
using EntryDate = FixTag<"272">;
using EntryTime = FixTag<"273">;
using EntryID = FixTag<"278">;
using EntryPosition = FixTag<"290">;
using TradingSessionID = FixTag<"336">;
using NumberOfOrders = FixTag<"346">;
using LastMsgSeqNumProcessed = FixTag<"369">;
using Username = FixTag<"553">;
using Password = FixTag<"554">;
using TradingSessionSubID = FixTag<"625">;
using NextExpectedSeqNum = FixTag<"789">;
using LastLiquidityIndicator = FixTag<"851">;
using NoFeedTypes = FixTag<"1021">;
using FeedType = FixTag<"1022">;
using PriceLevel = FixTag<"1023">;

enum class FixHeaderVersion {
   FIX42 = 0,
   FIX44,
   UNDEFINED
};

enum class MessageType : char {
   HEART_BEAT = '0',
   LOGON = 'A',
   LOGOUT = '5',
   MARKET_DATA_REUEST = 'V',
   MARKET_DATA = 'X',
   SNAPSHOT = 'W',
   NEW_ORDER_SINGLE = 'D'
};

MessageProtocol toFixProtocolEnum(const char* version);
std::string_view toFixProtocolString(MessageProtocol protocol);
::EntryType toCommonType(EntryType::Types type);
::UpdateAction toCommonType(UpdateAction::Types type);
::ResetSeqNumFlag toCommonType(ResetSeqNumFlag::Types type);
UpdateType::Types toFixType(::UpdateType type);
EntryType::Types toFixType(::EntryType type);
ResetSeqNumFlag::Types toFixType(::ResetSeqNumFlag type);
SubscriptionRequestType::Types toFixType(::SubscriptionRequestType type);
MessageType toFixType(::MessageType type);
::MessageType toCommonType(MessageType type);
Side::Types toFixType(::OrderSide type);
OrderType::Types toFixType(::OrderType type);
TimeInForce::Types toFixType(::TimeInForce type);

}

#endif