#ifndef ORDER_MANAGER_H
#define ORDER_MANAGER_H

#include <vector>
#include <atomic>
#include <variant>
#include <array>
#include "CommonUtils.h"
#include "SymbolState.h"
#include "ExecutionEvents.h"

class OrderEntry
{
public:
    OrderEntry() = default;
    OrderEntry(OrderID orderId, StrategyID strategyId, OrderID strategyOrderId, OrderSide side, SymbolID symbolId, Price price, Volume orderVolume, OrderType type, TimeInForce timeInForce) : 
        orderId_(orderId), strategyId_(strategyId), strategyOrderId_(strategyOrderId), side_(side), symbolId_(symbolId), price_(price), orderVolume_(orderVolume), type_(type), timeInForce_(timeInForce), status_(static_cast<int>(OrderStatus::PENDING_NEW)) {}
    void orderId(OrderID orderId) { orderId_ = orderId; }
    void setStrategyId(StrategyID strategyId) { strategyId_ = strategyId; }
    void setStrategyOrderId(OrderID strategyOrderId) { strategyOrderId_ = strategyOrderId; }
    void setSide(OrderSide side) { side_ = side; }
    void setStatus(OrderStatus status) { status_ = static_cast<int>(status); }
    void addStatus(OrderStatus status) { status_ |= static_cast<int>(status); }
    void removeStatus(OrderStatus status) { status_ &= ~static_cast<int>(status); }
    void setFilledVolume(Volume filledVolume) { filledVolume_ = filledVolume; }
    void setVolume(Volume volume) { orderVolume_ = volume; }
    void setPrice(Price price) { price_ = price; }
    void setOrderType(OrderType type) { type_ = type; }
    void setTimeInForce(TimeInForce timeInForce) { timeInForce_ = timeInForce; }
    void setStopPrice(Price stopPrice) { stopPrice_ = stopPrice; }
    void setNewPrice(Price newPrice) { newPrice_ = newPrice;}
    void setNewVolume(Volume newVolume) { newVolume_ = newVolume;}
    void setNewStopPrice(Price newStopPrice) { newStopPrice_ = newStopPrice; }
    void setNewTimeInForce(TimeInForce newTimeInForce) { newTimeInForce_ = newTimeInForce; }
    void setBrokerOrderId(const std::array<char,50>& brokerOrderID) { brokerOrderId_ = brokerOrderID; }
    OrderID getOrderId() const { return orderId_; }
    StrategyID getStrategyId() const { return strategyId_; }
    OrderID getStrategyOrderId() const { return strategyOrderId_; }
    OrderSide getSide() const { return side_; }
    SymbolID getSymbolId() const { return symbolId_; }
    Price getPrice() const { return price_; }
    Volume getOrderVolume() const { return orderVolume_; }
    OrderType getOrderType() const { return type_; }
    TimeInForce getTimeInForce() const { return timeInForce_; }
    Price getStopPrice() const { return stopPrice_; }
    Volume getFilledVolume() const { return filledVolume_; }
    Price getNewPrice() const { return newPrice_; }
    Volume getNewVolume() const { return newVolume_; }
    Price getNewStopPrice() const { return newStopPrice_; }
    TimeInForce getNewTimeInForce() const { return newTimeInForce_; }
    int getStatus() const { return status_; }
    const std::array<char,50>& getBrokerOrderId() const { return brokerOrderId_; }
private:
    OrderID orderId_ = 0;
    StrategyID strategyId_ = NoStrategyID;
    OrderID strategyOrderId_ = 0;
    OrderSide side_ = OrderSide::UNKNOWN;
    SymbolID symbolId_ = NoSymbolID;
    Price price_ = 0;
    Volume orderVolume_ = 0;
    OrderType type_ = OrderType::UNKNOWN;
    TimeInForce timeInForce_ = TimeInForce::UNKNOWN;
    Price stopPrice_ = 0;
    Volume filledVolume_ = 0;
    Price newPrice_ = 0;
    Volume newVolume_ = 0;
    Price newStopPrice_ = 0;
    TimeInForce newTimeInForce_ = TimeInForce::UNKNOWN;
    std::array<char,50> brokerOrderId_;
    int status_ = static_cast<int>(OrderStatus::UNKNOWN);
};

std::ostream& operator<<(std::ostream &os, const OrderEntry& order);

class BrokerProfile;

class OrderManager
{
    friend std::ostream& operator<<(std::ostream &os, const OrderManager& om);
public:
    OrderManager(std::size_t orderEntrySlots, std::size_t maxOrders, std::vector<SymbolState> &symbolStates_, BrokerProfile& brokerProfile);
    OrderManager(const OrderManager& om) = delete;
    OrderManager& operator=(const OrderManager& om) = delete;
    std::size_t handleEvent(const SingleOrderEvent& event);
    std::size_t handleEvent(const CancelOrderEvent& event);
    std::size_t handleEvent(const EditOrderEvent& event);
    bool handleEvent(const ExecutionReportEvent& event);
    bool handleEvent(const OrderCancelRejectEvent& event);
    inline const OrderEntry* getOrderEntry(std::size_t id) const ;
    void registerForOrderManagerEvent(Subscriber subscriber) { orderManagerEventListeners_.push_back(subscriber); }
private:
    bool handleSingleOrderAckEvent(const ExecutionReportMessage& msg);
    bool handleSingleOrderRejectEvent(const ExecutionReportMessage& msg);
    bool handleOrderFillEvent(const ExecutionReportMessage& msg);
    bool handleCancelOrderRejectEvent(const OrderCancelRejectMessage& msg);
    bool handleCanceledOrder(const ExecutionReportMessage& msg);
    bool handleEditOrderRejectEvent(const OrderCancelRejectMessage& msg);
    bool handleEditOrderAckEvent(const ExecutionReportMessage& msg);
    inline OrderEntry* getEditableOrderEntry(std::size_t i);
    template<typename T> void notifyOrderManagerEventListners(const T& event);
    void emitBrokerStatus();
    void emitSymbolStatus(const SymbolState& state);
private:
    std::size_t orderEntryEnd_;
    std::vector<OrderEntry> orderEntries_;
    std::vector<OrderID> orderIDVsEntryMap_;
    std::vector<OrderID> freeEntries_;
    OrderID orderEntrySlots_;
    OrderID maxOrders_;
    OrderID freeEntryPtr_ = 0;
    OrderID currentOrderID_ =  1;
    std::size_t ownShares_ = 0;
    std::vector<Volume> pendingShares_;
    std::vector<Volume> pendingCancellations_;
    std::vector<Price> pendingShareValues_;
    std::vector<SymbolState> &symbolStates_;
    BrokerProfile& brokerProfile_;
    std::vector<Subscriber> orderManagerEventListeners_;
};

const OrderEntry* OrderManager::getOrderEntry(std::size_t id) const
{
    if (id > maxOrders_)
        return nullptr;

    auto orderEntryptr = orderIDVsEntryMap_[id];

    if (orderEntryptr == orderEntryEnd_)
        return nullptr;

    return &orderEntries_[orderEntryptr];
}

OrderEntry* OrderManager::getEditableOrderEntry(std::size_t id)
{
    if (id > maxOrders_)
        return nullptr;

    auto orderEntryptr = orderIDVsEntryMap_[id];

    if (orderEntryptr == orderEntryEnd_)
        return nullptr;

    return &orderEntries_[orderEntryptr];
}

template<typename T> void OrderManager::notifyOrderManagerEventListners(const T& event)
{
    for (auto& subscriber : orderManagerEventListeners_) {
        subscriber.notify(event);
    }
}

std::ostream& operator<<(std::ostream &os, const OrderManager& om);
#endif