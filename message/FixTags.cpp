#include "FixTags.h"
#include "TypeDef.h"

namespace Fix
{

MessageProtocol toFixProtocolEnum(const char* version) 
{
   if (!strcmp(version, "FIX.4.2"))
      return MessageProtocol::FIX42;
   if (!strcmp(version, "FIX.4.4"))
      return MessageProtocol::FIX44;
   return MessageProtocol::UNKNOWN;
}

std::string_view toFixProtocolString(MessageProtocol protocol)
{
   switch (protocol) {
      case MessageProtocol::FIX42:
         return "FIX.4.2";
      case MessageProtocol::FIX44:
         return "FIX.4.4";
   }
}

::EntryType toCommonType(EntryType::Types type)
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

::UpdateAction toCommonType(UpdateAction::Types type)
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

::ResetSeqNumFlag toCommonType(ResetSeqNumFlag::Types type)
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

UpdateType::Types toFixType(::UpdateType type)
{
   switch (type) {
      case ::UpdateType::FULL_REFRESH:
         return UpdateType::Types::FULL_REFRESH;
      case ::UpdateType::INCREMENTAL_REFRESH:
         return UpdateType::Types::INCREMENTAL_REFRESH;
   }

}


EntryType::Types toFixType(::EntryType type)
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

ResetSeqNumFlag::Types toFixType(::ResetSeqNumFlag type)
{
   switch (type) {
      case ::ResetSeqNumFlag::NO:
         return ResetSeqNumFlag::Types::NO;
      case ::ResetSeqNumFlag::YES:
         return ResetSeqNumFlag::Types::YES;
   }
}

SubscriptionRequestType::Types toFixType(::SubscriptionRequestType type)
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

 MessageType toFixType(::MessageType type)
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
      case ::MessageType::NEW_ORDER_SINGLE:
         return MessageType::NEW_ORDER_SINGLE;
   }
}

::MessageType toCommonType(MessageType type)
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

Side::Types toFixType(::OrderSide type)
{
   switch (type) {
      case ::OrderSide::BUY:
         return Side::Types::BUY;
      case ::OrderSide::SELL:
         return Side::Types::SELL;
   }
}

OrderType::Types toFixType(::OrderType type)
{
   switch (type) {
      case ::OrderType::MARKET:
         return OrderType::Types::MARKET;
      case ::OrderType::LIMIT:
         return OrderType::Types::LIMIT;
      case ::OrderType::STOP:
         return OrderType::Types::STOP;
      case ::OrderType::STOP_LIMIT:
         return OrderType::Types::STOP_LIMIT;
   }
}

TimeInForce::Types toFixType(::TimeInForce type)
{
   switch (type) {
      case ::TimeInForce::DAY:
         return TimeInForce::Types::DAY;
      case ::TimeInForce::GOOD_TILL_CANCEL:
         return TimeInForce::Types::GOOD_TILL_CANCEL;
      case ::TimeInForce::AT_THE_OPENING:
         return TimeInForce::Types::AT_THE_OPENING;
      case ::TimeInForce::IMMEDIATE_OR_CANCEL:
         return TimeInForce::Types::IMMEDIATE_OR_CANCEL;
      case ::TimeInForce::FILL_OR_KILL:
         return TimeInForce::Types::FILL_OR_KILL;
      case ::TimeInForce::GOOD_TILL_CROSSING:
         return TimeInForce::Types::GOOD_TILL_CROSSING;
      case ::TimeInForce::GOOD_TILL_DATE:
         return TimeInForce::Types::GOOD_TILL_DATE;
   }
}

}