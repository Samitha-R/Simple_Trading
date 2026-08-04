#ifndef FIX_TAGS_H
#define FIX_TAGS_H

#include <charconv>
#include <cstdint>
#include <vector>
#include <string>
#include <cstring>
#include <array>

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
      UNDEFINED
   };
};

struct SubscriptionRequestType : public FixTag<"263"> {
   enum class Types {
      SNAPSHOT = 0,
      SNAPSHOT_AND_UPDATE,
      CANCEL,
      UNDEFINED
   };
};

struct EntryType : public FixTag<"269"> {
   enum class Types {
      BID = '0',
      OFFER = '1',
      TRADE = '2',
      UNDEFINED
   };
};

struct UpdateAction : public FixTag<"279"> {
   enum class Types {
      NEW = 0,
      CHANGE,
      DELETE,
      UNDEFINED
   };
};

struct AggressorIndicator : public FixTag<"5797"> {
   enum class Types {
      PASSIVE = 'N',
      AGGRESSOR = 'Y',
      UNDEFINED
   };
};

struct UpdateType : public FixTag<"265"> {
   enum class Types {
      FULL_REFRESH = 0,
      INCREMENTAL_REFRESH,
      UNDEFINED
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

enum class FixVersion {
   FIX42 = 0,
   FIX44,
   UNDEFINED
};

static const std::array<const char*, static_cast<int>(FixVersion::UNDEFINED)> fixVersonStringValues = {"FIX.4.2", "FIX.4.4"};
inline std::string_view toString(FixVersion version) { return fixVersonStringValues[static_cast<int>(version)];}

inline FixVersion toEnum(const char* version) 
{
   for (auto i = 0; i < fixVersonStringValues.size(); ++i) {
      if (!std::strcmp(fixVersonStringValues[i], version)) {
         return static_cast<FixVersion>(i);
      }
   }
   return FixVersion::UNDEFINED;
}

#endif