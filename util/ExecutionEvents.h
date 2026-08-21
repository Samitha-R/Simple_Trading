#ifndef EXECUTION_EVENTS
#define EXECUTION_EVENTS

#include "Events.h"
#include "TypeDef.h"

class SingleOrderEvent : public EventBase
{
public:
    enum class Type {
        BUY,
        SELL,
        UNDEFINED
    };
public:
    SingleOrderEvent() : EventBase(EventType::SINGLE_ORDER) {}
    SingleOrderEvent(BrokerID brokerId, SymbolID symbol, Type type, Price price, Volume volume) : EventBase(EventType::SINGLE_ORDER), 
        brokerId_(brokerId), symbol_(symbol), type_(type), price_(price), volume_(volume) {}
    SymbolID getSymbolId() const { return symbol_; }
    BrokerID getBrokerId() const { return brokerId_; }
    Price getPrice() const { return price_; }
    Volume getVolume() const { return volume_; }
    Type getType() const { return type_; }
private:
    BrokerID brokerId_ = NoBrokerID;
    SymbolID symbol_ = NoSymbolID;
    Type type_ = Type::UNDEFINED;
    Price price_ = 0;
    Volume volume_ = 0;
};

class CancelOrderEvent : public EventBase
{
public:
    CancelOrderEvent() : EventBase(EventType::CANCEL_ORDER) {}
    CancelOrderEvent(BrokerID brokerId, SymbolID symbol, std::size_t orderId) : EventBase(EventType::CANCEL_ORDER), 
        brokerId_(brokerId), orderId_(orderId) {}
    BrokerID getBrokerId() const { return brokerId_; }
    std::size_t getOrderId() const { return orderId_; }
private:
    BrokerID brokerId_ = NoBrokerID;
    std::size_t orderId_ = 0;

};

class EditOrderEvent : public EventBase
{
public:
    EditOrderEvent() : EventBase(EventType::EDIT_ORDER) {}
    EditOrderEvent(BrokerID brokerId, std::size_t orderId, Price newPrice, Volume newVolume) : EventBase(EventType::EDIT_ORDER), 
        brokerId_(brokerId), orderId_(orderId), newPrice_(newPrice), newVolume_(newVolume) {}
    BrokerID getBrokerId() const { return brokerId_; }
    std::size_t getOrderId() const { return orderId_; }
    Price getNewPrice() const { return newPrice_; }
    Volume getNewVolume() const { return newVolume_; }
private:
    BrokerID brokerId_ = NoBrokerID;
    std::size_t orderId_ = 0;
    Price newPrice_ = 0;
    Volume newVolume_ = 0;
};

class OrderFillEvent : public EventBase
{
public:
    OrderFillEvent() : EventBase(EventType::ORDER_FILL) {}
    OrderFillEvent(BrokerID brokerId, std::size_t orderId, Price price, Volume filledVolume, Volume comVolume, Volume remainingVolume) : 
        EventBase(EventType::ORDER_FILL), brokerId_(brokerId), orderId_(orderId), price_(price), filledVolume_(filledVolume), 
        comVolume_(comVolume), remainingVolume_(remainingVolume) {}
    BrokerID getBrokerId() const { return brokerId_; }
    std::size_t getOrderId() const { return orderId_; }
    Price getPrice() const { return price_; }
    Volume getFilledVolume() const { return filledVolume_; }
    Volume getComVolume() const { return comVolume_; }
    Volume getRemainingVolume() const { return remainingVolume_; }
private:
    BrokerID brokerId_ = NoBrokerID;
    std::size_t orderId_ = 0;
    Price price_ = 0;
    Volume filledVolume_ = 0;
    Volume comVolume_ = 0;
    Volume remainingVolume_ = 0;
};

class SingleOrderAckEvent : public EventBase
{
public:
    SingleOrderAckEvent() : EventBase(EventType::SINGLE_ORDER_ACK) {}
    SingleOrderAckEvent(BrokerID brokerId, std::size_t orderId) : EventBase(EventType::SINGLE_ORDER_ACK), brokerId_(brokerId), orderId_(orderId) {}
    BrokerID getBrokerId() const { return brokerId_; }
    std::size_t getOrderId() const { return orderId_; }
private:
    BrokerID brokerId_ = NoBrokerID;
    std::size_t orderId_ = 0;
};

class SingleOrderRejectEvent : public EventBase
{
public:
    SingleOrderRejectEvent() : EventBase(EventType::SINGLE_ORDER_REJECT) {}
    SingleOrderRejectEvent(BrokerID brokerId, std::size_t orderId) : EventBase(EventType::SINGLE_ORDER_REJECT), brokerId_(brokerId), orderId_(orderId) {}
    BrokerID getBrokerId() const { return brokerId_; }
    std::size_t getOrderId() const { return orderId_; }
private:
    BrokerID brokerId_ = NoBrokerID;
    std::size_t orderId_ = 0;
};

class CancelOrderAckEvent : public EventBase
{
public:
    CancelOrderAckEvent() : EventBase(EventType::CANCEL_ORDER_ACK) {}
    CancelOrderAckEvent(BrokerID brokerId, std::size_t orderId) : EventBase(EventType::CANCEL_ORDER_ACK), brokerId_(brokerId), orderId_(orderId) {}
    BrokerID getBrokerId() const { return brokerId_; }
    std::size_t getOrderId() const { return orderId_; }
private:
    BrokerID brokerId_ = NoBrokerID;
    std::size_t orderId_ = 0;
};

class CancelOrderRejectEvent : public EventBase
{
public:
    CancelOrderRejectEvent() : EventBase(EventType::CANCEL_ORDER_REJECT) {}
    CancelOrderRejectEvent(BrokerID brokerId, std::size_t orderId) : EventBase(EventType::CANCEL_ORDER_REJECT), brokerId_(brokerId), orderId_(orderId) {}
    BrokerID getBrokerId() const { return brokerId_; }
    std::size_t getOrderId() const { return orderId_; }
private:
    BrokerID brokerId_ = NoBrokerID;
    std::size_t orderId_ = 0;
};

class EditOrderAckEvent : public EventBase
{public:
    EditOrderAckEvent() : EventBase(EventType::EDIT_ORDER_ACK) {}    
    EditOrderAckEvent(BrokerID brokerId, std::size_t orderId) : EventBase(EventType::EDIT_ORDER_ACK), brokerId_(brokerId), orderId_(orderId) {}
    BrokerID getBrokerId() const { return brokerId_; }
    std::size_t getOrderId() const { return orderId_; }
private:
    BrokerID brokerId_ = NoBrokerID;
    std::size_t orderId_ = 0;
};

class EditOrderRejectEvent : public EventBase
{
    public:
    EditOrderRejectEvent() : EventBase(EventType::EDIT_ORDER_REJECT) {}
    EditOrderRejectEvent(BrokerID brokerId, std::size_t orderId) : EventBase(EventType::EDIT_ORDER_REJECT), brokerId_(brokerId), orderId_(orderId) {}
    BrokerID getBrokerId() const { return brokerId_; }
    std::size_t getOrderId() const { return orderId_; }
private:
    BrokerID brokerId_ = NoBrokerID;
    std::size_t orderId_ = 0;
};

class OrderFailureInfo
{
public:
    enum class Type {
        BUY,
        SELL,
        UNDEFINED
    };

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
    OrderFailureInfo(BrokerID brokerId, SymbolID symbolId, Type type, Price price, Volume volume, Volume filledVolume, FailureReason reason) : 
        brokerId_(brokerId), symbolId_(symbolId), type_(type), price_(price), volume_(volume), filledVolume_(filledVolume), 
        reason_(reason) {}
    BrokerID getBrokerId() const { return brokerId_; }
    SymbolID getSymbolId() const { return symbolId_; }
    Type getType() const { return type_; }
    Price getPrice() const { return price_; }
    Volume getVolume() const { return volume_; }
    Volume getFilledVolume() const { return filledVolume_; }
    FailureReason getFailureReason() const { return reason_; }
protected:
    BrokerID brokerId_ = NoBrokerID;
    SymbolID symbolId_ = NoSymbolID;
    Type type_ = Type::UNDEFINED;
    Price price_ = 0;
    Volume volume_ = 0;
    Volume filledVolume_ = 0;
    FailureReason reason_ = FailureReason::UNKNOWN_ERROR;
};

class SingleOrderFailureEvent : public OrderFailureInfo, public EventBase
{
public:
    SingleOrderFailureEvent(BrokerID brokerId, SymbolID symbolId, Type type, Price price, Volume volume, Volume filledVolume, FailureReason reason) : OrderFailureInfo(brokerId, symbolId, type, price, volume, filledVolume, reason), EventBase(EventType::SINGLE_ORDER_FAILURE) {}
};

class CancelOrderFailureEvent : public OrderFailureInfo, public EventBase
{
public:
    CancelOrderFailureEvent(BrokerID brokerId, SymbolID symbolId, Type type, Price price, Volume volume, Volume filledVolume, FailureReason reason) : OrderFailureInfo(brokerId, symbolId, type, price, volume, filledVolume, reason), EventBase(EventType::CANCEL_ORDER_FAILURE) {}
};

class EditOrderFailureEvent : public OrderFailureInfo, public EventBase
{
public:
    EditOrderFailureEvent(BrokerID brokerId, SymbolID symbolId, Type type, Price price, Volume volume, Volume filledVolume, FailureReason reason, Volume newVolume, Price newPrice) : OrderFailureInfo(brokerId, symbolId, type, price, volume, filledVolume, reason), EventBase(EventType::EDIT_ORDER_FAILURE), newVolume_(newVolume), newPrice_(newPrice) {}

public:
     Volume getNewVolume() const { return newVolume_; }
     Price getNewPrice() const { return newPrice_; }
private:
    Volume newVolume_ = 0;
    Price newPrice_ = 0;
};

#endif