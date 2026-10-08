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

using BeginString = FixTag<"8">;
using BodyLength = FixTag<"9">;
using CheckSum = FixTag<"10">;
using SecuirtyIDSource = FixTag<"22">;
using LastPrice = FixTag<"31">;
using LastQuantity = FixTag<"32">;
using MsgSeqNum = FixTag<"34">;
using MsgType = FixTag<"35">;
using PossDupFlag = FixTag<"43">;
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
};

inline MessageProtocol toFixProtocolEnum(const char* version) 
{
   if (!strcmp(version, "FIX.4.2"))
      return MessageProtocol::FIX42;
   if (!strcmp(version, "FIX.4.4"))
      return MessageProtocol::FIX44;
   return MessageProtocol::UNKNOWN;
}

inline std::string_view toFixProtocolString(MessageProtocol protocol)
{
   switch (protocol) {
      case MessageProtocol::FIX42:
         return "FIX.4.2";
      case MessageProtocol::FIX44:
         return "FIX.4.4";
   }
}
inline ::EntryType toCommonType(EntryType::Types type)
{
   switch (type) {
      case EntryType::Types::BID:
         return ::EntryType::BID;
      case EntryType::Types::OFFER:
         return ::EntryType::OFFER;
      case EntryType::Types::TRADE:
         return ::EntryType::TRADE;
      default:
         return ::EntryType::UNKNOWN;
   }
}

inline ::UpdateAction toCommonType(UpdateAction::Types type)
{
   switch (type) {
      case UpdateAction::Types::NEW:
         return ::UpdateAction::NEW;
      case UpdateAction::Types::CHANGE:
         return ::UpdateAction::CHANGE;
      case UpdateAction::Types::DELETE:
         return ::UpdateAction::DELETE;
      default:
         return ::UpdateAction::UNKNOWN;
   }  

}

inline ::ResetSeqNumFlag toCommonType(ResetSeqNumFlag::Types type)
{
   switch (type) {
      case ResetSeqNumFlag::Types::NO:
         return ::ResetSeqNumFlag::NO;
      case ResetSeqNumFlag::Types::YES:
         return ::ResetSeqNumFlag::YES;
      default:
         return ::ResetSeqNumFlag::UNKNOWN;
   }
}

inline UpdateType::Types toFixType(::UpdateType type)
{
   switch (type) {
      case ::UpdateType::FULL_REFRESH:
         return UpdateType::Types::FULL_REFRESH;
      case ::UpdateType::INCREMENTAL_REFRESH:
         return UpdateType::Types::INCREMENTAL_REFRESH;
   }

}


inline EntryType::Types toFixType(::EntryType type)
{
   switch (type) {
      case ::EntryType::BID:
         return EntryType::Types::BID;
      case ::EntryType::OFFER:
         return EntryType::Types::OFFER;
      case ::EntryType::TRADE:
         return EntryType::Types::TRADE;
   }
}

inline ResetSeqNumFlag::Types toFixType(::ResetSeqNumFlag type)
{
   switch (type) {
      case ::ResetSeqNumFlag::NO:
         return ResetSeqNumFlag::Types::NO;
      case ::ResetSeqNumFlag::YES:
         return ResetSeqNumFlag::Types::YES;
   }
}

inline SubscriptionRequestType::Types toFixType(::SubscriptionRequestType type)
{
   switch (type) {
      case ::SubscriptionRequestType::SNAPSHOT:
         return SubscriptionRequestType::Types::SNAPSHOT;
      case ::SubscriptionRequestType::SNAPSHOT_AND_UPDATE:
         return SubscriptionRequestType::Types::SNAPSHOT_AND_UPDATE;
      case ::SubscriptionRequestType::CANCEL:
         return SubscriptionRequestType::Types::CANCEL;
   }
}

inline MessageType toFixType(::MessageType type)
{
   switch (type) {
      case ::MessageType::HEART_BEAT:
         return MessageType::HEART_BEAT;
      case ::MessageType::LOGON:
         return MessageType::LOGON;
      case ::MessageType::LOGOUT:
         return MessageType::LOGOUT;
      case ::MessageType::MARKET_DATA_REUEST:
         return MessageType::MARKET_DATA_REUEST;
      case ::MessageType::MARKET_DATA:
         return MessageType::MARKET_DATA;
      case ::MessageType::SNAPSHOT:
         return MessageType::SNAPSHOT;
   }
}

inline ::MessageType toCommonType(MessageType type)
{
   switch (type) {
      case MessageType::HEART_BEAT:
         return ::MessageType::HEART_BEAT;
      case MessageType::LOGON:
         return ::MessageType::LOGON;
      case MessageType::LOGOUT:
         return ::MessageType::LOGOUT;
      case MessageType::MARKET_DATA_REUEST:
         return ::MessageType::MARKET_DATA_REUEST;
      case MessageType::MARKET_DATA:
         return ::MessageType::MARKET_DATA;
      case MessageType::SNAPSHOT:
         return ::MessageType::SNAPSHOT;
   }
}

}

#endif