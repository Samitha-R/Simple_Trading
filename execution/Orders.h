#ifndef ORDERS_H
#define ORDERS_H

#include "MarketData.h"

struct BaseOrder
{
    enum class Type { BUY, SELL, EDIT, CANCEL, UNDEFINED};
    enum class Status {NEW, ACKNOWLEDGED, PARTIALLY_COMPLETE, REJECTED, COMPLETE};
    BaseOrder() = default;
    BaseOrder(Type type, std::size_t orderId) : type_(type), orderId_(orderId) {}
    Type type_ = Type::UNDEFINED;
    Status status_ = Status::NEW;
    std::size_t orderId_ = 0;
};

struct SingleOrder : BaseOrder
{
    SingleOrder() = default;
    SingleOrder(SymbolID symbolId, Type type, std::size_t orderId, Price price, Volume orderVolume) : BaseOrder(type, orderId), symbolId_(symbolId), price_(price), orderVolume_(orderVolume) {}
    SymbolID symbolId_ = 0;
    Price price_ = 0;
    Volume orderVolume_ = 0;
    Volume filledVolume_ = 0;
    std::size_t childOriderId_ = 0;
};

struct CancelOrder : BaseOrder
{
    CancelOrder() : BaseOrder(Type::CANCEL, 0){}
    CancelOrder(std::size_t orderId, std::size_t parentOrderId) : BaseOrder(Type::CANCEL, orderId), parentOriderId_(parentOrderId) {}
    std::size_t parentOriderId_ = 0;
};

struct EditOrder : BaseOrder
{
    EditOrder() : BaseOrder(Type::EDIT, 0) {}
    EditOrder(std::size_t orderId, std::size_t parentOrderId, Price price, Volume orderVolume) : 
        BaseOrder(Type::EDIT, orderId), price_(price), orderVolume_(orderVolume), parentOriderId_(parentOrderId) {} 
    Price price_ = 0;
    Volume orderVolume_ = 0;
    std::size_t parentOriderId_ = 0;
};

enum class OrderFailureReason
{
    NONE,
    INSUFFICIENT_BALANCE,
    ORDER_SIZE_EXCEEDS_LIMIT,
    PRICE_DEVIATION_TOO_HIGH,
    ORDER_RATE_LIMIT_EXCEEDED
};

#endif