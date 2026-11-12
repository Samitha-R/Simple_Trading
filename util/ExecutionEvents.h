#ifndef EXECUTION_EVENTS
#define EXECUTION_EVENTS

#include "Events.h"
#include "TypeDef.h"
#include "Message.h"

class SingleOrderEvent : public EventBase
{
public:
    SingleOrderEvent() : EventBase(EventType::SINGLE_ORDER) {}
    SingleOrderEvent(BrokerID brokerId, StrategyID strategyID, OrderID strategyOrderId, std::size_t clientOrderID, SymbolID symbolID, OrderSide side, std::chrono::system_clock::time_point orderTime, Volume orderVolume, Price orderPrice, OrderType orderType, TimeInForce timeInForce) : EventBase(EventType::SINGLE_ORDER), 
        brokerId_(brokerId), strategyID_(strategyID), strategyOrderId_(strategyOrderId), symbolID_(symbolID), side_(side), orderTime_(orderTime), orderVolume_(orderVolume), orderPrice_(orderPrice), orderType_(orderType), timeInForce_(timeInForce) {}
    void setBrokerID(BrokerID brokerId) { brokerId_ = brokerId; }
    void setStrategyID(StrategyID strategyID) { strategyID_ = strategyID; }
    void setStraegyOrderID(OrderID strategyOrderId) { strategyOrderId_ = strategyOrderId; }
    void setSymbolID(SymbolID symbolID) { symbolID_ = symbolID; }
    void setSide(OrderSide side) { side_ = side; }
    void setOrderTime(std::chrono::system_clock::time_point orderTime) { orderTime_ = orderTime; }
    void setOrderVolume(Volume orderVolume) { orderVolume_ = orderVolume; }
    void setOrderPrice(Price orderPrice) { orderPrice_ = orderPrice; }
    void setStopPrice(Price stopPrice) {stopPrice_ = stopPrice; }
    void setOrderType(OrderType orderType) { orderType_ = orderType; }
    void setTimeInForce(TimeInForce timeInForce) { timeInForce_ = timeInForce; }
    BrokerID getBrokerId() const { return brokerId_; }
    StrategyID getStrategyID() const { return strategyID_; }
    OrderID getStrategyOrderID() const { return strategyOrderId_;}
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
    StrategyID strategyID_ = NoStrategyID;
    OrderID strategyOrderId_ = 0;
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
    CancelOrderEvent(BrokerID brokerId, StrategyID strategyID, OrderID strategyOrderID, OrderID orderId) : EventBase(EventType::CANCEL_ORDER),
        brokerId_(brokerId), strategyID_(strategyID), strategyOrderId_(strategyOrderID), orderId_(orderId), orderTime_(std::chrono::system_clock::time_point()) {}
    BrokerID getBrokerId() const { return brokerId_; }
    StrategyID getStrategyID() const { return strategyID_; }
    OrderID getOrderId() const { return orderId_; }
    OrderID getStrategyOrderID() const { return strategyOrderId_; }
    std::chrono::system_clock::time_point getOrderTime() const { return orderTime_; }
private:
    BrokerID brokerId_ = NoBrokerID;
    StrategyID strategyID_ = NoStrategyID;
    OrderID strategyOrderId_ = 0;
    OrderID orderId_ = 0;
    std::chrono::system_clock::time_point orderTime_;
};

class EditOrderEvent : public EventBase
{
public:
    EditOrderEvent() : EventBase(EventType::EDIT_ORDER) {}
    EditOrderEvent(BrokerID brokerId, StrategyID strategyID, OrderID strategyOrderId, OrderID orderId, Price newPrice, Volume newVolume) : EventBase(EventType::EDIT_ORDER),
        brokerId_(brokerId), strategyID_(strategyID), strategyOrderId_(strategyOrderId), orderId_(orderId), newPrice_(newPrice), newVolume_(newVolume), orderTime_(std::chrono::system_clock::time_point()) {}
    void setStopPrice(Price stopPrice) { stopPrice_ = stopPrice; }
    void setTimeInForce(TimeInForce timeInForce) { timeInForce_ = timeInForce; }
    BrokerID getBrokerId() const { return brokerId_; }
    StrategyID getStrategyID() const { return strategyID_; }
    OrderID getStrategyOrderID() const { return strategyOrderId_; }
    OrderID getOrderId() const { return orderId_; }
    Price getNewPrice() const { return newPrice_; }
    Volume getNewVolume() const { return newVolume_; }
    Price getStopPrice() const { return stopPrice_; }
    TimeInForce getTimeInForce() const { return timeInForce_; }
private:
    BrokerID brokerId_ = NoBrokerID;
    StrategyID strategyID_ = NoStrategyID;
    OrderID strategyOrderId_ = 0;
    OrderID orderId_ = 0;
    Price newPrice_ = 0;
    Volume newVolume_ = 0;
    Price stopPrice_ = 0;
    TimeInForce timeInForce_ = TimeInForce::UNKNOWN;
    std::chrono::system_clock::time_point orderTime_;
};

class ExecutionReportEvent : public EventBase
{
public:
    ExecutionReportEvent() : EventBase(EventType::EXECUTION_REPORT) { }
    ExecutionReportEvent(BrokerID brokerId, const ExecutionReportMessage& msg) : EventBase(EventType::EXECUTION_REPORT), brokerId_(brokerId), msg_(msg) {}
    const ExecutionReportMessage& getMessage() const { return msg_; }
    BrokerID getBrokerId() const { return brokerId_; }
private:
    BrokerID brokerId_ = NoBrokerID;
    ExecutionReportMessage msg_;
};

class OrderCancelRejectEvent : public EventBase
{
public:
    OrderCancelRejectEvent() : EventBase(EventType::ORDER_CANCEL_REJECT) { }
    OrderCancelRejectEvent(BrokerID brokerId, const OrderCancelRejectMessage& msg) : EventBase(EventType::ORDER_CANCEL_REJECT), brokerId_(brokerId), msg_(msg) {}
    const OrderCancelRejectMessage& getMessage() const { return msg_; }
    BrokerID getBrokerId() const { return brokerId_; }
private:
    BrokerID brokerId_ = NoBrokerID;
    OrderCancelRejectMessage msg_;
};

class BrokerStatusEvent : public EventBase
{
public:
    BrokerStatusEvent() : EventBase(EventType::BROKER_STATUS) {}
    BrokerStatusEvent(BrokerID brokerID, Price currentBalance) : EventBase(EventType::BROKER_STATUS), brokerID_(brokerID), currentBalance_(currentBalance) {}
    BrokerID getBrokerID() const { return brokerID_; }
    Price getCurrentBalance() const { return currentBalance_; }
private:
    BrokerID brokerID_ = NoBrokerID;
    Price currentBalance_ = Price(0);
};

class SymbolStatusEvent : public EventBase 
{
public:
    SymbolStatusEvent() : EventBase(EventType::SYMBOL_STATUS) {}
    SymbolStatusEvent(SymbolID symbolID, Volume ownShares, Price avgBuyPrice, Price bestBidPrice, std::array<Volume,2> pendingShares, std::array<Volume,2> pendingCancellations) : EventBase(EventType::SYMBOL_STATUS), symbolId_(symbolID), ownShares_(ownShares), avgBuyPrice_(avgBuyPrice), bestBidPrice_(bestBidPrice), pendingShares_(pendingShares), pendingCancellations_(pendingCancellations) {}
    SymbolID getSymbolID() const { return symbolId_; }
    Volume getOwnShares() const { return ownShares_; }
    Price getAvgBuyPrice() const { return avgBuyPrice_; }
    Price getBestBidPrice() const { return bestBidPrice_; }
    const std::array<Volume,2>& getPendingShares() const { return pendingShares_; }
    const std::array<Volume,2>& getPendingCancellations() const { return pendingCancellations_; } 
private:
    SymbolID symbolId_ = NoSymbolID;
    Volume maxAllowedShares_ = 0;
    Volume ownShares_ = 0;
    Price avgBuyPrice_ = 0;
    Price bestBidPrice_ = 0;
    std::array<Volume,2> pendingShares_;
    std::array<Volume,2> pendingCancellations_;
};

enum class OrderFailureReason {
    UNKNOWN,
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

class OrderStatusEvent : public EventBase
{
public:
    OrderStatusEvent() : EventBase(EventType::ORDER_STATUS) {}
    OrderStatusEvent(StrategyID strategyId, OrderID strategyOrderId, OrderID execOrderID, Price price, Volume volume, Volume filledVolume, int status) :
     EventBase(EventType::ORDER_STATUS), strategyId_(strategyId), strategyOrderId_(strategyOrderId), execOrderID_(execOrderID), pirce_(price), volume_(volume), filledVolume_(filledVolume), status_(status) {}
    void setOrderFailureReason(OrderFailureReason reason) { failureReason_ = reason; }
    StrategyID getStrategyID() const { return strategyId_; }
    OrderID getStrategyOrderID() const { return strategyOrderId_; }
    OrderID getExecOrderID() const { return execOrderID_; }
    Price getPrice() const { return pirce_; }
    Volume getVolume() const { return volume_; } 
    Volume getFilledVolume() const { return filledVolume_; }
    int getStatus() const { return status_; }
    OrderFailureReason getOrderFailureReason() const { return failureReason_; }
private:
    StrategyID strategyId_ = NoStrategyID;
    OrderID strategyOrderId_ = 0;
    OrderID execOrderID_ = 0;
    Price pirce_ = 0;
    Volume volume_ = 0;
    Volume filledVolume_ = 0;
    int status_ = static_cast<int>(OrderStatus::UNKNOWN);
    OrderFailureReason failureReason_ = OrderFailureReason::UNKNOWN;

};

class OrderReplaceFailureEvent : public EventBase
{
public:
    OrderReplaceFailureEvent() : EventBase(EventType::ORDER_REPLACE_FAILURE) {}
    OrderReplaceFailureEvent(StrategyID strategyId, OrderID strategyOrderId, OrderID execOrderID, OrderFailureReason failureReason) :
     EventBase(EventType::ORDER_REPLACE_FAILURE), strategyId_(strategyId), strategyOrderId_(strategyOrderId), execOrderID_(execOrderID), failureReason_(failureReason) {}
    StrategyID getStrategyID() const { return strategyId_; }
    OrderID getStrategyOrderID() const { return strategyOrderId_; }
    OrderID getExecOrderID() const { return execOrderID_; }
    OrderFailureReason getFailureReason() const { return failureReason_; }
private:
    StrategyID strategyId_ = NoStrategyID;
    OrderID strategyOrderId_ = 0;
    OrderID execOrderID_ = 0;
    OrderFailureReason failureReason_ = OrderFailureReason::UNKNOWN;
};

#endif