#ifndef ORDER_MANAGER_H
#define ORDER_MANAGER_H

#include <vector>
#include <atomic>
#include <variant>
#include "CommonUtils.h"
#include "Orders.h"
#include "SymbolState.h"
#include "ExecutionEvents.h"

using CommonOrder = std::variant<SingleOrder, CancelOrder, EditOrder>;

class BrokerProfile;

class OrderManager
{
public:
    OrderManager(std::size_t orderEntrySlots, std::size_t maxOrders, std::vector<SymbolState> &symbolStates_, BrokerProfile& brokerProfile);
    OrderManager(const OrderManager& om) = delete;
    OrderManager& operator=(const OrderManager& om) = delete;
    std::size_t handleEvent(const SingleOrderEvent& event);
    std::size_t handleEvent(const SingleOrderAckEvent& event);
    void handleEvent(const SingleOrderRejectEvent& event);
    std::size_t handleEvent(const CancelOrderEvent& event);
    std::size_t handleEvent(const EditOrderEvent& event);
    bool handleEvent(const OrderFillEvent& event);
    void handleEvent(const CancelOrderRejectEvent& event);
    void handleEvent(const CancelOrderAckEvent& event);
    void handleEvent(const EditOrderRejectEvent& event);
    void handleEvent(const EditOrderAckEvent& event);
    inline const CommonOrder* getOrderEntry(std::size_t id) const ;
    std::size_t getUpdateCount() const { return updateCount_.load(std::memory_order_acquire); }
    void registerForOrderManagerEvent(Subscriber subscriber) { orderManagerEventListeners_.push_back(subscriber); }
private:
    inline CommonOrder* getEditableOrderEntry(std::size_t i);
    template<typename T> void notifyOrderManagerEventListners(const T& event);
private:
    BrokerID brokerId_ = NoBrokerID;
    std::size_t orderEntryEnd_;
    std::vector<CommonOrder> orderEntries_;
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
    std::vector<Volume> pendingShareValues_;
    std::vector<SymbolState> &symbolStates_;
    BrokerProfile& brokerProfile_;
    std::vector<Subscriber> orderManagerEventListeners_;
};

const CommonOrder* OrderManager::getOrderEntry(std::size_t id) const
{
    if (id > maxOrders_)
        return nullptr;

    auto orderEntryptr = orderIDVsEntryMap_[id];

    if (orderEntryptr == orderEntryEnd_)
        return nullptr;

    return &orderEntries_[orderEntryptr];
}

CommonOrder* OrderManager::getEditableOrderEntry(std::size_t id)
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

#endif