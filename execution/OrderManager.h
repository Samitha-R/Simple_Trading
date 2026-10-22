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
    OrderEntry(std::size_t orderId, OrderSide side, SymbolID symbolId, Price price, Volume orderVolume) : 
        orderId_(orderId), side_(side), symbolId_(symbolId), price_(price), orderVolume_(orderVolume), status_(static_cast<int>(OrderState::PENDING_NEW)) {}
    void setStatus(OrderState status) { status_ = static_cast<int>(status); }
    void addStatus(OrderState status) { status_ |= static_cast<int>(status); }
    void removeStatus(OrderState status) { status_ &= ~static_cast<int>(status); }
    void setFilledVolume(Volume filledVolume) { filledVolume_ = filledVolume; }
    void setVolume(Volume volume) { orderVolume_ = volume; }
    void setPrice(Price price) { price_ = price; }
    void setNewPrice(Price newPrice) { newPrice_ = newPrice;}
    void setNewVolume(Volume newVolume) { newVolume_ = newVolume;}
    std::size_t getOrderId() const { return orderId_; }
    OrderSide getSide() const { return side_; }
    SymbolID getSymbolId() const { return symbolId_; }
    Price getPrice() const { return price_; }
    Volume getOrderVolume() const { return orderVolume_; }
    Volume getFilledVolume() const { return filledVolume_; }
    Price getNewPrice() const { return newPrice_; }
    Volume getNewVolume() const { return newVolume_; }
    int getStatus() const { return status_; }
private:
    std::size_t orderId_ = 0;
    OrderSide side_ = OrderSide::UNKNOWN;
    SymbolID symbolId_ = NoSymbolID;
    Price price_ = 0;
    Volume orderVolume_ = 0;
    Volume filledVolume_ = 0;
    Price newPrice_ = 0;
    Volume newVolume_ = 0;
    int status_ = static_cast<int>(OrderState::UNKONWN);
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
    bool handleEvent(const SingleOrderAckEvent& event);
    bool handleEvent(const SingleOrderRejectEvent& event);
    std::size_t handleEvent(const CancelOrderEvent& event);
    std::size_t handleEvent(const EditOrderEvent& event);
    bool handleEvent(const OrderFillEvent& event);
    bool handleEvent(const CancelOrderRejectEvent& event);
    bool handleEvent(const CancelOrderAckEvent& event);
    bool handleEvent(const EditOrderRejectEvent& event);
    bool handleEvent(const EditOrderAckEvent& event);
    inline const OrderEntry* getOrderEntry(std::size_t id) const ;
    std::size_t getUpdateCount() const { return updateCount_.load(std::memory_order_acquire); }
    void registerForOrderManagerEvent(Subscriber subscriber) { orderManagerEventListeners_.push_back(subscriber); }
private:
    inline OrderEntry* getEditableOrderEntry(std::size_t i);
    template<typename T> void notifyOrderManagerEventListners(const T& event);
private:
    std::size_t orderEntryEnd_;
    std::vector<OrderEntry> orderEntries_;
    std::vector<std::size_t> orderIDVsEntryMap_;
    std::vector<std::size_t> freeEntries_;
    std::size_t orderEntrySlots_;
    std::size_t maxOrders_;
    std::size_t freeEntryPtr_ = 0;
    std::size_t currentOrderID_ =  1;
    std::atomic<std::size_t> updateCount_;
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