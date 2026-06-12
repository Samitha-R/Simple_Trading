#include "OrderManager.h"

OrderManager::OrderManager(std::size_t orderEntrySlots, std::size_t maxOrders, std::vector<SymbolState> &symbolStates_, ExecutionEventNotifier& eventNotifier) : 
    orderEntryEnd_(orderEntrySlots + 1), orderEntries_(orderEntrySlots), orderIDVsEntryMap_(maxOrders, orderEntryEnd_), 
    freeEntries_(orderEntrySlots), orderEntrySlots_(orderEntrySlots), maxOrders_(maxOrders), pendingShares_(2,0), pendingCancellations_(2,0),
     pendingShareValues_(2,0), symbolStates_(symbolStates_), eventNotifier_(eventNotifier)
{
    for (std::size_t i = 0; i < orderEntrySlots; ++i) {
        freeEntries_[i] = i;
    }
    updateCount_.store(0, std::memory_order_release);
}

std::size_t OrderManager::addSingleOrder(const SingleOrderEvent& event)
{
    if (currentOrderID_ > maxOrders_) {
        auto orderType = event.getType() == SingleOrderEvent::Type::BUY ? SingleOrderFailureEvent::Type::BUY : SingleOrderFailureEvent::Type::SELL;
        SingleOrderFailureEvent failureEvent(event.getBrokerId(), event.getSymbolId(), orderType, event.getPrice(), event.getVolume(),
         0, SingleOrderFailureEvent::FailureReason::DAILY_LIMIT_EXCEEDED);
        eventNotifier_.notifySingleOrderFailure(failureEvent);
        return 0;
    }

    if (freeEntryPtr_ >= orderEntrySlots_) {
        auto orderType = event.getType() == SingleOrderEvent::Type::BUY ? SingleOrderFailureEvent::Type::BUY : SingleOrderFailureEvent::Type::SELL;
        SingleOrderFailureEvent failureEvent(event.getBrokerId(), event.getSymbolId(), orderType, event.getPrice(), event.getVolume(),
         0, SingleOrderFailureEvent::FailureReason::NO_SPACE_TO_SAVE_ORDER);
        eventNotifier_.notifySingleOrderFailure(failureEvent);
        return 0;
    }

    BaseOrder::Type type = event.getType() == SingleOrderEvent::Type::BUY ? BaseOrder::Type::BUY : BaseOrder::Type::SELL;
    SingleOrder order(event.getSymbolId(), type , currentOrderID_, event.getPrice(), event.getVolume());
    auto freeEntryIndex = freeEntries_[freeEntryPtr_];
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    orderIDVsEntryMap_[currentOrderID_] = freeEntryIndex;
    orderEntries_[freeEntryIndex] = order;
    pendingShares_[static_cast<std::size_t>(type)] += order.orderVolume_;
    ++freeEntryPtr_;
    ++currentOrderID_;
    symbolStates_[event.getSymbolId()].addSingleOrder(event.getBrokerId(), type, order.orderVolume_, order.price_);
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    return order.orderId_;
}

std::size_t OrderManager::onSingleOrderAck(const SingleOrderAckEvent& event)
{
    auto comOrder = getEditableOrderEntry(event.getOrderId());

    if (!comOrder)
        return 0;

    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    auto &order = std::get<SingleOrder>(*comOrder);
    order.status_ = BaseOrder::Status::ACKNOWLEDGED;
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    return order.orderId_;
}

void OrderManager::onSingleOrderReject(const SingleOrderRejectEvent& event)
{
    auto comOrder = getEditableOrderEntry(event.getOrderId());

    if (!comOrder)
        return;

    auto &order = std::get<SingleOrder>(*comOrder);
    auto orderType = order.type_ == BaseOrder::Type::BUY ? SingleOrderFailureEvent::Type::BUY : SingleOrderFailureEvent::Type::SELL;
    SingleOrderFailureEvent failureEvent(event.getBrokerId(), order.symbolId_, orderType, order.price_, order.orderVolume_,
     order.filledVolume_, SingleOrderFailureEvent::FailureReason::BROKER_REJECTION);

    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    order.status_ = BaseOrder::Status::REJECTED;
    auto unfilledVolume = order.orderVolume_ - order.filledVolume_;
    pendingShares_[static_cast<std::size_t>(order.type_)] -= unfilledVolume;
    auto entryIndex = orderIDVsEntryMap_[order.orderId_];
    --freeEntryPtr_;
    freeEntries_[freeEntryPtr_] = entryIndex;
    symbolStates_[order.symbolId_].onSingleOrderReject(event.getBrokerId(), order.type_, order.price_, unfilledVolume);
    updateCount_.fetch_add(1, std::memory_order_acq_rel);

    eventNotifier_.notifySingleOrderFailure(failureEvent);
}

bool OrderManager::onFilledVolume(const OrderFillEvent& event)
{
    auto comOrder = getEditableOrderEntry(event.getOrderId());

    if (!comOrder)
        return false;

    auto &order = std::get<SingleOrder>(*comOrder);
    auto comCountLocal = order.filledVolume_ + event.getFilledVolume();

    if ((event.getComVolume() != comCountLocal) || (event.getRemainingVolume() + event.getComVolume() != order.orderVolume_))
        return false;

    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    order.filledVolume_ = comCountLocal;

    if (order.type_ == BaseOrder::Type::BUY) {
        ownShares_ += event.getFilledVolume();
    } else {
        ownShares_ -= event.getFilledVolume();
    }

    pendingShares_[static_cast<std::size_t>(order.type_)] -= event.getFilledVolume();
    pendingShareValues_[static_cast<std::size_t>(order.type_)] -= event.getFilledVolume() * order.price_;
    symbolStates_[order.symbolId_].onOrderFill(event.getBrokerId(), order.type_, event.getFilledVolume(), order.price_);

    if (order.filledVolume_ == order.orderVolume_) {
        order.status_ = BaseOrder::Status::COMPLETE;

        if (!order.childOriderId_) {
            auto entryIndex = orderIDVsEntryMap_[order.orderId_];
            --freeEntryPtr_;
            freeEntries_[freeEntryPtr_] = entryIndex;
        } 
    }

    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    
    return true;
}

std::size_t OrderManager::addSingleOrderCancel(const CancelOrderEvent& event)
{
    auto comOrder = getEditableOrderEntry(event.getOrderId());

    if (!comOrder)
        return 0;

    auto &parentOrder = std::get<SingleOrder>(*comOrder);

    if (parentOrder.childOriderId_) {
        return 0;
    }

    if (currentOrderID_ > maxOrders_) {
        auto orderType = parentOrder.type_ == BaseOrder::Type::BUY ? SingleOrderFailureEvent::Type::BUY : SingleOrderFailureEvent::Type::SELL;
        SingleOrderFailureEvent failureEvent(event.getBrokerId(), parentOrder.symbolId_, orderType, parentOrder.price_, parentOrder.orderVolume_,
         parentOrder.filledVolume_, SingleOrderFailureEvent::FailureReason::DAILY_LIMIT_EXCEEDED);
        eventNotifier_.notifySingleOrderFailure(failureEvent);
        return 0;
    }

    if (freeEntryPtr_ >= orderEntrySlots_) {
        auto orderType = parentOrder.type_ == BaseOrder::Type::BUY ? SingleOrderFailureEvent::Type::BUY : SingleOrderFailureEvent::Type::SELL;
        SingleOrderFailureEvent failureEvent(event.getBrokerId(), parentOrder.symbolId_, orderType, parentOrder.price_, parentOrder.orderVolume_, parentOrder.filledVolume_,
         SingleOrderFailureEvent::FailureReason::NO_SPACE_TO_SAVE_ORDER);
        eventNotifier_.notifySingleOrderFailure(failureEvent);
        return 0;
    }

    CancelOrder order(currentOrderID_, event.getOrderId());
    auto freeEntryIndex = freeEntries_[freeEntryPtr_];
    auto cancelVolume = parentOrder.orderVolume_ - parentOrder.filledVolume_;
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    pendingCancellations_[static_cast<std::size_t>(parentOrder.type_)] += cancelVolume;
    parentOrder.childOriderId_ = currentOrderID_;
    orderIDVsEntryMap_[order.orderId_] = freeEntryIndex;
    orderEntries_[freeEntryIndex] = order;
    ++freeEntryPtr_;
    ++currentOrderID_;
    symbolStates_[parentOrder.symbolId_].addSingleOrderCancel(brokerId_, parentOrder.type_, parentOrder.price_, cancelVolume);
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    return 0;
}

std::size_t OrderManager::addSingleOrderEdit(const EditOrderEvent& event)
{
    auto comOrder = getEditableOrderEntry(event.getOrderId());

    if (!comOrder)
        return 0;

    auto &parentOrder = std::get<SingleOrder>(*comOrder);

    if (parentOrder.childOriderId_ )
    
    if (currentOrderID_ > maxOrders_) {
        auto orderType = parentOrder.type_ == BaseOrder::Type::BUY ? EditOrderFailureEvent::Type::BUY : EditOrderFailureEvent::Type::SELL;
        EditOrderFailureEvent failureEvent(event.getBrokerId(), parentOrder.symbolId_, orderType, parentOrder.price_, parentOrder.orderVolume_, parentOrder.filledVolume_,
         EditOrderFailureEvent::FailureReason::DAILY_LIMIT_EXCEEDED, event.getNewPrice(), event.getNewVolume());
        eventNotifier_.notifyEditOrderFailure(failureEvent);
        return 0;
    }

    if (freeEntryPtr_ >= orderEntrySlots_) {
        auto orderType = parentOrder.type_ == BaseOrder::Type::BUY ? EditOrderFailureEvent::Type::BUY : EditOrderFailureEvent::Type::SELL;
        EditOrderFailureEvent failureEvent(event.getBrokerId(), parentOrder.symbolId_, orderType, parentOrder.price_, parentOrder.orderVolume_, parentOrder.filledVolume_,
         EditOrderFailureEvent::FailureReason::NO_SPACE_TO_SAVE_ORDER, event.getNewPrice(), event.getNewVolume());
        eventNotifier_.notifyEditOrderFailure(failureEvent);
        return 0;
    }

    if (event.getNewVolume() < parentOrder.filledVolume_) {
        auto orderType = parentOrder.type_ == BaseOrder::Type::BUY ? EditOrderFailureEvent::Type::BUY : EditOrderFailureEvent::Type::SELL;
        EditOrderFailureEvent failureEvent(event.getBrokerId(), parentOrder.symbolId_, orderType, parentOrder.price_, parentOrder.orderVolume_, parentOrder.filledVolume_,
         EditOrderFailureEvent::FailureReason::EDIT_VOLUME_LESS_THAN_FILLED_VOLUME, event.getNewPrice(), event.getNewVolume());
        eventNotifier_.notifyEditOrderFailure(failureEvent);
        return 0;
    }

    EditOrder order(currentOrderID_, event.getOrderId(), event.getNewPrice(), event.getNewVolume());
    auto freeEntryIndex = freeEntries_[freeEntryPtr_];
    auto orderVolumeChange = event.getNewVolume() - parentOrder.orderVolume_;

    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    pendingShares_[static_cast<std::size_t>(parentOrder.type_)] += orderVolumeChange;
    parentOrder.childOriderId_ = currentOrderID_;
    orderIDVsEntryMap_[order.orderId_] = freeEntryIndex;
    orderEntries_[freeEntryIndex] = order;
    ++freeEntryPtr_;
    ++currentOrderID_;
    symbolStates_[parentOrder.symbolId_].addSingleOrderEdit(brokerId_, parentOrder.type_, parentOrder.price_, event.getNewPrice(), parentOrder.orderVolume_, event.getNewVolume());
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    return order.orderId_;
}

void  OrderManager::onCancelOrderReject(const CancelOrderRejectEvent& event)
{
    auto comOrder = getEditableOrderEntry(event.getOrderId());

    if (!comOrder)
        return;

    auto &cancelOrder = std::get<CancelOrder>(*comOrder);
    auto parentComOrder = getEditableOrderEntry(cancelOrder.parentOriderId_);

    if (!parentComOrder)
        return;

    auto &parentOrder = std::get<SingleOrder>(*parentComOrder);
    auto orderType = parentOrder.type_ == BaseOrder::Type::BUY ? CancelOrderFailureEvent::Type::BUY : CancelOrderFailureEvent::Type::SELL;
    CancelOrderFailureEvent failureEvent(event.getBrokerId(), parentOrder.symbolId_, orderType, parentOrder.price_, parentOrder.orderVolume_, parentOrder.filledVolume_, CancelOrderFailureEvent::FailureReason::BROKER_REJECTION);

    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    auto cancelledVolume = parentOrder.orderVolume_ - parentOrder.filledVolume_;
    pendingCancellations_[static_cast<std::size_t>(parentOrder.type_)] -= cancelledVolume;
    parentOrder.childOriderId_ = 0;
    auto entryIndex = orderIDVsEntryMap_[cancelOrder.orderId_];
    --freeEntryPtr_;
    freeEntries_[freeEntryPtr_] = entryIndex;
    symbolStates_[parentOrder.symbolId_].onCancelOrderReject(brokerId_, parentOrder.type_, parentOrder.price_, cancelledVolume);
    updateCount_.fetch_add(1, std::memory_order_acq_rel);

    eventNotifier_.notifyCancelOrderFailure(failureEvent);
}

void OrderManager::onCancelOrderAck(const CancelOrderAckEvent& event)
{
    auto comOrder = getEditableOrderEntry(event.getOrderId());

    if (!comOrder)
        return;

    auto &cancelOrder = std::get<CancelOrder>(*comOrder);
    auto parentComOrder = getEditableOrderEntry(cancelOrder.parentOriderId_);

    if (!parentComOrder)
        return;

    auto &parentOrder = std::get<SingleOrder>(*parentComOrder);
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    auto cancelledVolume = parentOrder.orderVolume_ - parentOrder.filledVolume_;
    pendingCancellations_[static_cast<std::size_t>(parentOrder.type_)] -= cancelledVolume;
    pendingShares_[static_cast<std::size_t>(parentOrder.type_)] -= cancelledVolume;
    auto entryIndex = orderIDVsEntryMap_[cancelOrder.orderId_];
    --freeEntryPtr_;
    freeEntries_[freeEntryPtr_] = entryIndex;

    entryIndex = orderIDVsEntryMap_[cancelOrder.parentOriderId_];
    --freeEntryPtr_;
    freeEntries_[freeEntryPtr_] = entryIndex;
    symbolStates_[parentOrder.symbolId_].onCancelOrderAck(brokerId_, parentOrder.type_, parentOrder.price_, cancelledVolume);
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
}

void OrderManager::onEditOrderReject(const EditOrderRejectEvent& event)
{
    auto comOrder = getEditableOrderEntry(event.getOrderId());

    if (!comOrder)
        return;

    auto &editOrder = std::get<EditOrder>(*comOrder);
    auto parentComOrder = getEditableOrderEntry(editOrder.parentOriderId_);

    if (!parentComOrder)
        return;

    auto &parentOrder = std::get<SingleOrder>(*parentComOrder);
    auto orderType = parentOrder.type_ == BaseOrder::Type::BUY ? EditOrderFailureEvent::Type::BUY : EditOrderFailureEvent::Type::SELL;
    EditOrderFailureEvent failureEvent(event.getBrokerId(), parentOrder.symbolId_, orderType, parentOrder.price_, parentOrder.orderVolume_, parentOrder.filledVolume_, EditOrderFailureEvent::FailureReason::BROKER_REJECTION, editOrder.price_, editOrder.orderVolume_);

    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    auto orderVolumeChange = editOrder.orderVolume_ - parentOrder.orderVolume_;
    pendingShares_[static_cast<std::size_t>(parentOrder.type_)] -= orderVolumeChange;
    parentOrder.childOriderId_ = 0;
    auto entryIndex = orderIDVsEntryMap_[editOrder.orderId_];
    --freeEntryPtr_;
    freeEntries_[freeEntryPtr_] = entryIndex;
    symbolStates_[parentOrder.symbolId_].onEditOrderReject(brokerId_, parentOrder.type_, parentOrder.price_, editOrder.price_, parentOrder.orderVolume_, editOrder.orderVolume_);
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    eventNotifier_.notifyEditOrderFailure(failureEvent);
}

void OrderManager::onEditOrderAck(const EditOrderAckEvent& event)
{
    auto comOrder = getEditableOrderEntry(event.getOrderId());

    if (!comOrder)
        return;

    auto &editOrder = std::get<EditOrder>(*comOrder);
    auto parentComOrder = getEditableOrderEntry(editOrder.parentOriderId_);

    if (!parentComOrder)
        return;

    auto &parentOrder = std::get<SingleOrder>(*parentComOrder);
    SingleOrder newOrder(event.getBrokerId(), parentOrder.type_, event.getOrderId(), editOrder.price_, editOrder.orderVolume_);
    newOrder.filledVolume_ = parentOrder.filledVolume_;

    updateCount_.fetch_add(1, std::memory_order_acq_rel);

    auto entryIndex = orderIDVsEntryMap_[parentOrder.orderId_];
    --freeEntryPtr_;
    freeEntries_[freeEntryPtr_] = entryIndex;

    orderIDVsEntryMap_[event.getOrderId()] = newOrder.orderId_;
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
}

void OrderManager::setBalance(Price amount)
{
    balance_ = amount;
}

void OrderManager::credit(Price amount)
{
    balance_ += amount;
}

bool OrderManager::debit(Price amount)
{
     if (balance_ >= amount) {
        balance_ -= amount;
        return true;
    }
    return false;
}