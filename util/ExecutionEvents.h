#ifndef EXECUTION_EVENTS
#define EXECUTION_EVENTS

#include "Events.h"
#include "TypeDef.h"
#include "Message.h"

class SingleOrderEvent : public EventBase
{
public:
    SingleOrderEvent() : EventBase(EventType::SINGLE_ORDER) {}
    SingleOrderEvent(BrokerID brokerId, std::size_t clientOrderID, SymbolID symbolID, OrderSide side, std::chrono::system_clock::time_point orderTime, Volume orderVolume, Price orderPrice, OrderType orderType, TimeInForce timeInForce) : EventBase(EventType::SINGLE_ORDER), 
        symbolID_(symbolID), side_(side), orderTime_(orderTime), orderVolume_(orderVolume), orderPrice_(orderPrice), orderType_(orderType), timeInForce_(timeInForce), brokerId_(brokerId) {}
    BrokerID getBrokerId() const { return brokerId_; }
    void setSymbolID(SymbolID symbolID) { symbolID_ = symbolID; }
    void setSide(OrderSide side) { side_ = side; }
    void setOrderTime(std::chrono::system_clock::time_point orderTime) { orderTime_ = orderTime; }
    void setOrderVolume(Volume orderVolume) { orderVolume_ = orderVolume; }
    void setOrderPrice(Price orderPrice) { orderPrice_ = orderPrice; }
    void setStopPrice(Price stopPrice) {stopPrice_ = stopPrice; }
    void setOrderType(OrderType orderType) { orderType_ = orderType; }
    void setTimeInForce(TimeInForce timeInForce) { timeInForce_ = timeInForce; }
    SymbolID getSymbolID() const { return symbolID_; }
    OrderSide getSide() const { return side_; }
    std::chrono::system_clock::time_point getOrderTime() const { return orderTime_; }
    Volume getOrderVolume() const { return orderVolume_; }
    Price getOrderPrice() const { return orderPrice_; }
    Price getStopPrice() const { return stopPrice_; }
    OrderType getOrderType() const { return orderType_; }
    TimeInForce getTimeInForce() const { return timeInForce_; }
private:
    BrokerID brokerId_ = NoBrokerID;
    SymbolID symbolID_ = NoSymbolID;
    OrderSide side_ = OrderSide::UNKNOWN;
    std::chrono::system_clock::time_point orderTime_;
    Volume orderVolume_ = 0;
    Price orderPrice_ = 0;
    Price stopPrice_ = 0;
    OrderType orderType_ = OrderType::UNKNOWN;
    TimeInForce timeInForce_ = TimeInForce::UNKNOWN;
};

class CancelOrderEvent : public EventBase
{
public:
    CancelOrderEvent() : EventBase(EventType::CANCEL_ORDER) {}
    CancelOrderEvent(BrokerID brokerId, SymbolID symbol, std::size_t orderId, std::chrono::system_clock::time_point orderTime) : EventBase(EventType::CANCEL_ORDER), 
        brokerId_(brokerId), orderId_(orderId), orderTime_(orderTime) {}
    BrokerID getBrokerId() const { return brokerId_; }
    std::size_t getOrderId() const { return orderId_; }
    std::chrono::system_clock::time_point getOrderTime();
private:
    BrokerID brokerId_ = NoBrokerID;
    std::size_t orderId_ = 0;
    std::chrono::system_clock::time_point orderTime_;
};

class EditOrderEvent : public EventBase
{
public:
    EditOrderEvent() : EventBase(EventType::EDIT_ORDER) {}
    EditOrderEvent(BrokerID brokerId, std::size_t orderId, Price newPrice, Volume newVolume,  std::chrono::system_clock::time_point orderTime) : EventBase(EventType::EDIT_ORDER), 
        brokerId_(brokerId), orderId_(orderId), newPrice_(newPrice), newVolume_(newVolume), orderTime_(orderTime) {}
    void setStopPrice(Price stopPrice) { stopPrice_ = stopPrice; }
    void setTimeInForce(TimeInForce timeInForce) { timeInForce_ = timeInForce; }
    BrokerID getBrokerId() const { return brokerId_; }
    std::size_t getOrderId() const { return orderId_; }
    Price getNewPrice() const { return newPrice_; }
    Volume getNewVolume() const { return newVolume_; }
    Price getStopPrice() const { return stopPrice_; }
    TimeInForce getTimeInForce() const { return timeInForce_; }
private:
    BrokerID brokerId_ = NoBrokerID;
    std::size_t orderId_ = 0;
    Price newPrice_ = 0;
    Volume newVolume_ = 0;
    Price stopPrice_ = 0;
    TimeInForce timeInForce_ = TimeInForce::UNKNOWN;
    std::chrono::system_clock::time_point orderTime_;
};

class ExecutionReportEvent : public EventBase
{
public:
    ExecutionReportEvent() : EventBase(EventType::EXECUTION_REPORT_EVENT) { }
    ExecutionReportEvent(BrokerID brokerId, const ExecutionReportMessage& msg) : EventBase(EventType::EXECUTION_REPORT_EVENT), brokerId_(brokerId), msg_(msg) {}
    const ExecutionReportMessage& getMessage() const { return msg_; }
    BrokerID getBrokerId() const { return brokerId_; }
private:
    BrokerID brokerId_ = NoBrokerID;
    ExecutionReportMessage msg_;
};

class OrderCancelRejectEvent : public EventBase
{
public:
    OrderCancelRejectEvent() : EventBase(EventType::ORDER_CANCEL_REJECT_EVENT) { }
    OrderCancelRejectEvent(BrokerID brokerId, const OrderCancelRejectMessage& msg) : EventBase(EventType::ORDER_CANCEL_REJECT_EVENT), brokerId_(brokerId), msg_(msg) {}
    const OrderCancelRejectMessage& getMessage() const { return msg_; }
    BrokerID getBrokerId() const { return brokerId_; }
private:
    BrokerID brokerId_ = NoBrokerID;
    OrderCancelRejectMessage msg_;
};

class OrderFailureInfo
{
public:
    enum class FailureReason {
        UNKNOWN_ERROR,
        INSUFFICIENT_FUNDS,
        DAILY_LIMIT_EXCEEDED,
        ORDER_RATE_LIMIT_EXCEEDED,
        ORDER_SIZE_NOT_WITHIN_LIMITS,
        NO_SPACE_TO_SAVE_ORDER,
        ORDER_NOT_IN_BOOK,
        ORDER_ALREADY_CHANGED,
        BROKER_CLIENT_ORDER_MISMATCH,
        BROKER_REJECTION,
        EDIT_VOLUME_LESS_THAN_FILLED_VOLUME
    };
public:
    OrderFailureInfo() = default;
    OrderFailureInfo(BrokerID brokerId, SymbolID symbolId, OrderSide type, Price price, Volume volume, Volume filledVolume, FailureReason reason) : 
        brokerId_(brokerId), symbolId_(symbolId), type_(type), price_(price), volume_(volume), filledVolume_(filledVolume), 
        reason_(reason) {}
    BrokerID getBrokerId() const { return brokerId_; }
    SymbolID getSymbolId() const { return symbolId_; }
    OrderSide getType() const { return type_; }
    Price getPrice() const { return price_; }
    Volume getVolume() const { return volume_; }
    Volume getFilledVolume() const { return filledVolume_; }
    FailureReason getFailureReason() const { return reason_; }
protected:
    BrokerID brokerId_ = NoBrokerID;
    SymbolID symbolId_ = NoSymbolID;
    OrderSide type_ = OrderSide::UNKNOWN;
    Price price_ = 0;
    Volume volume_ = 0;
    Volume filledVolume_ = 0;
    FailureReason reason_ = FailureReason::UNKNOWN_ERROR;
};

class SingleOrderFailureEvent : public OrderFailureInfo, public EventBase
{
public:
    SingleOrderFailureEvent(BrokerID brokerId, SymbolID symbolId, OrderSide type, Price price, Volume volume, Volume filledVolume, FailureReason reason) : OrderFailureInfo(brokerId, symbolId, type, price, volume, filledVolume, reason), EventBase(EventType::SINGLE_ORDER_FAILURE) {}
};

class CancelOrderFailureEvent : public OrderFailureInfo, public EventBase
{
public:
    CancelOrderFailureEvent(BrokerID brokerId, SymbolID symbolId, OrderSide type, Price price, Volume volume, Volume filledVolume, FailureReason reason) : OrderFailureInfo(brokerId, symbolId, type, price, volume, filledVolume, reason), EventBase(EventType::CANCEL_ORDER_FAILURE) {}
};

class EditOrderFailureEvent : public OrderFailureInfo, public EventBase
{
public:
    EditOrderFailureEvent(BrokerID brokerId, SymbolID symbolId, OrderSide type, Price price, Volume volume, Volume filledVolume, FailureReason reason, Volume newVolume, Price newPrice) : OrderFailureInfo(brokerId, symbolId, type, price, volume, filledVolume, reason), EventBase(EventType::EDIT_ORDER_FAILURE), newVolume_(newVolume), newPrice_(newPrice) {}

public:
     Volume getNewVolume() const { return newVolume_; }
     Price getNewPrice() const { return newPrice_; }
private:
    Volume newVolume_ = 0;
    Price newPrice_ = 0;
};

#endif