#ifndef ORDER_MANAGER_H
#define ORDER_MANAGER_H

#include <vector>
#include <atomic>
#include <variant>
#include "CommonUtils.h"
#include "Orders.h"
#include "SymbolState.h"
#include "ExecutionEvents.h"
#include "ExecutionEventNotifier.h"

using CommonOrder = std::variant<SingleOrder, CancelOrder, EditOrder>;

class OrderManager
{
public:
    OrderManager(std::size_t orderEntrySlots, std::size_t maxOrders, std::vector<SymbolState> &symbolStates_, ExecutionEventNotifier& eventNotifier);
    OrderManager(const OrderManager& om) = delete;
    OrderManager& operator=(const OrderManager& om) = delete;
    std::size_t addSingleOrder(const SingleOrderEvent& event);
    std::size_t onSingleOrderAck(const SingleOrderAckEvent& event);
    void onSingleOrderReject(const SingleOrderRejectEvent& event);
    std::size_t addSingleOrderCancel(const CancelOrderEvent& event);
    std::size_t addSingleOrderEdit(const EditOrderEvent& event);
    bool onFilledVolume(const OrderFillEvent& event);
    void onCancelOrderReject(const CancelOrderRejectEvent& event);
    void onCancelOrderAck(const CancelOrderAckEvent& event);
    void onEditOrderReject(const EditOrderRejectEvent& event);
    void onEditOrderAck(const EditOrderAckEvent& event);
    void setBalance(Price balance);
    Price getBalance() const { return balance_; }
    void credit(Price amount);
    bool debit(Price amount);
    inline const CommonOrder* getOrderEntry(std::size_t id) const ;
    std::size_t getUpdateCount() const { return updateCount_.load(std::memory_order_acquire); }
private:
    inline CommonOrder* getEditableOrderEntry(std::size_t i);
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
    Price balance_ = 0;
    ExecutionEventNotifier& eventNotifier_;
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

#endif