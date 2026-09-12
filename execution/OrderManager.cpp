#include "OrderManager.h"
#include "BrokerProfile.h"

OrderManager::OrderManager(std::size_t orderEntrySlots, std::size_t maxOrders, std::vector<SymbolState> &symbolStates_, BrokerProfile& brokerProfile) : 
    orderEntryEnd_(orderEntrySlots + 1), orderEntries_(orderEntrySlots), orderIDVsEntryMap_(maxOrders, orderEntryEnd_), 
    freeEntries_(orderEntrySlots), orderEntrySlots_(orderEntrySlots), maxOrders_(maxOrders), pendingShares_(2,0), pendingCancellations_(2,0),
     pendingShareValues_(2,0), symbolStates_(symbolStates_), brokerProfile_(brokerProfile)
{
    orderManagerEventListeners_.reserve(3);

    for (std::size_t i = 0; i < orderEntrySlots; ++i) {
        freeEntries_[i] = i;
    }
    updateCount_.store(0, std::memory_order_release);
}

std::size_t OrderManager::handleEvent(const SingleOrderEvent& event)
{
    if (currentOrderID_ > maxOrders_) {
        SingleOrderFailureEvent failureEvent(event.getBrokerId(), event.getSymbolId(), event.getSide(), event.getPrice(), event.getVolume(),
        0, SingleOrderFailureEvent::FailureReason::DAILY_LIMIT_EXCEEDED);
        notifyOrderManagerEventListners(failureEvent);
        return 0;
    }

    if (freeEntryPtr_ >= orderEntrySlots_) {
        SingleOrderFailureEvent failureEvent(event.getBrokerId(), event.getSymbolId(), event.getSide(), event.getPrice(), event.getVolume(),
        0, SingleOrderFailureEvent::FailureReason::NO_SPACE_TO_SAVE_ORDER);
        notifyOrderManagerEventListners(failureEvent);
        return 0;
    }

    auto cost = brokerProfile_.calculateCost(event.getPrice(), event.getVolume(), event.getSide());

    if (!brokerProfile_.debitBalance(cost)) {
        // Reject order due to insufficient funds
        SingleOrderFailureEvent failureEvent(event.getBrokerId(), event.getSymbolId(), event.getSide(), event.getPrice(), event.getVolume(),
                                             0, SingleOrderFailureEvent::FailureReason::INSUFFICIENT_FUNDS);
        notifyOrderManagerEventListners(failureEvent);
        return 0;
    }

    SingleOrder order(event.getSymbolId(), event.getSide() , currentOrderID_, event.getPrice(), event.getVolume());
    auto freeEntryIndex = freeEntries_[freeEntryPtr_];
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    orderIDVsEntryMap_[currentOrderID_] = freeEntryIndex;
    orderEntries_[freeEntryIndex] = order;
    pendingShares_[static_cast<std::size_t>(order.side_)] += order.orderVolume_;
    ++freeEntryPtr_;
    ++currentOrderID_;
    symbolStates_[event.getSymbolId()].onSingleOrder(event.getBrokerId(), event.getSide(), order.orderVolume_, order.price_);
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    return order.orderId_;
}

std::size_t OrderManager::handleEvent(const SingleOrderAckEvent& event)
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

void OrderManager::handleEvent(const SingleOrderRejectEvent& event)
{
    auto comOrder = getEditableOrderEntry(event.getOrderId());

    if (!comOrder)
        return;

    auto &order = std::get<SingleOrder>(*comOrder);
    SingleOrderFailureEvent failureEvent(event.getBrokerId(), order.symbolId_, order.side_, order.price_, order.orderVolume_,
     order.filledVolume_, SingleOrderFailureEvent::FailureReason::BROKER_REJECTION);

    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    order.status_ = BaseOrder::Status::REJECTED;
    auto unfilledVolume = order.orderVolume_ - order.filledVolume_;
    pendingShares_[static_cast<std::size_t>(order.side_)] -= unfilledVolume;
    auto entryIndex = orderIDVsEntryMap_[order.orderId_];
    --freeEntryPtr_;
    freeEntries_[freeEntryPtr_] = entryIndex;
    symbolStates_[order.symbolId_].onSingleOrderReject(event.getBrokerId(), order.side_, order.price_, unfilledVolume);
    updateCount_.fetch_add(1, std::memory_order_acq_rel);

    auto creditAmount = brokerProfile_.calculateCost(order.price_, unfilledVolume, order.side_);
    brokerProfile_.creditBalance(creditAmount);

    notifyOrderManagerEventListners(failureEvent);
}

bool OrderManager::handleEvent(const OrderFillEvent& event)
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

    if (order.side_ == OrderSide::BUY) {
        ownShares_ += event.getFilledVolume();
    } else {
        ownShares_ -= event.getFilledVolume();
    }

    pendingShares_[static_cast<std::size_t>(order.type_)] -= event.getFilledVolume();
    pendingShareValues_[static_cast<std::size_t>(order.type_)] -= event.getFilledVolume() * order.price_;
    symbolStates_[order.symbolId_].onOrderFill(event.getBrokerId(), order.side_, event.getFilledVolume(), order.price_);

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

std::size_t OrderManager::handleEvent(const CancelOrderEvent& event)
{
    auto comOrder = getEditableOrderEntry(event.getOrderId());

    if (!comOrder)
        return 0;

    auto &parentOrder = std::get<SingleOrder>(*comOrder);

    if (parentOrder.childOriderId_) {
        return 0;
    }

    if (currentOrderID_ > maxOrders_) {
        SingleOrderFailureEvent failureEvent(event.getBrokerId(), parentOrder.symbolId_, parentOrder.side_, parentOrder.price_, parentOrder.orderVolume_,
         parentOrder.filledVolume_, SingleOrderFailureEvent::FailureReason::DAILY_LIMIT_EXCEEDED);
        notifyOrderManagerEventListners(failureEvent);
        return 0;
    }

    if (freeEntryPtr_ >= orderEntrySlots_) {
        SingleOrderFailureEvent failureEvent(event.getBrokerId(), parentOrder.symbolId_, parentOrder.side_, parentOrder.price_, parentOrder.orderVolume_, parentOrder.filledVolume_,
         SingleOrderFailureEvent::FailureReason::NO_SPACE_TO_SAVE_ORDER);
        notifyOrderManagerEventListners(failureEvent);
        return 0;
    }

    CancelOrder order(currentOrderID_, event.getOrderId());
    auto freeEntryIndex = freeEntries_[freeEntryPtr_];
    auto cancelVolume = parentOrder.orderVolume_ - parentOrder.filledVolume_;
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    pendingCancellations_[static_cast<std::size_t>(parentOrder.side_)] += cancelVolume;
    parentOrder.childOriderId_ = currentOrderID_;
    orderIDVsEntryMap_[order.orderId_] = freeEntryIndex;
    orderEntries_[freeEntryIndex] = order;
    ++freeEntryPtr_;
    ++currentOrderID_;
    symbolStates_[parentOrder.symbolId_].onSingleOrderCancel(brokerId_, parentOrder.side_, parentOrder.price_, cancelVolume);
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    return 0;
}

std::size_t OrderManager::handleEvent(const EditOrderEvent& event)
{
    auto comOrder = getEditableOrderEntry(event.getOrderId());

    if (!comOrder)
        return 0;

    auto &parentOrder = std::get<SingleOrder>(*comOrder);

    if (parentOrder.childOriderId_ )
        return 0;
    
    if (currentOrderID_ > maxOrders_) {
        EditOrderFailureEvent failureEvent(event.getBrokerId(), parentOrder.symbolId_, parentOrder.side_, parentOrder.price_, parentOrder.orderVolume_, parentOrder.filledVolume_,
         EditOrderFailureEvent::FailureReason::DAILY_LIMIT_EXCEEDED, event.getNewPrice(), event.getNewVolume());
        notifyOrderManagerEventListners(failureEvent);
        return 0;
    }

    if (freeEntryPtr_ >= orderEntrySlots_) {
        EditOrderFailureEvent failureEvent(event.getBrokerId(), parentOrder.symbolId_, parentOrder.side_, parentOrder.price_, parentOrder.orderVolume_, parentOrder.filledVolume_,
         EditOrderFailureEvent::FailureReason::NO_SPACE_TO_SAVE_ORDER, event.getNewPrice(), event.getNewVolume());
        notifyOrderManagerEventListners(failureEvent);
        return 0;
    }

    if (event.getNewVolume() < parentOrder.filledVolume_) {
        EditOrderFailureEvent failureEvent(event.getBrokerId(), parentOrder.symbolId_, parentOrder.side_, parentOrder.price_, parentOrder.orderVolume_, parentOrder.filledVolume_,
         EditOrderFailureEvent::FailureReason::EDIT_VOLUME_LESS_THAN_FILLED_VOLUME, event.getNewPrice(), event.getNewVolume());
        notifyOrderManagerEventListners(failureEvent);
        return 0;
    }

    auto unfilledVolume = parentOrder.orderVolume_ - parentOrder.filledVolume_;
    auto costForUnfilledVolume = brokerProfile_.calculateCost(parentOrder.price_, unfilledVolume, parentOrder.side_);
    auto newUnfilledVolume = event.getNewVolume() - parentOrder.filledVolume_;
    auto costForNewUnfilledVolume = brokerProfile_.calculateCost(event.getNewPrice(), newUnfilledVolume, parentOrder.side_);

    if (costForNewUnfilledVolume > costForUnfilledVolume) {

        auto additionalCost = costForNewUnfilledVolume - costForUnfilledVolume;

        if (!brokerProfile_.debitBalance(additionalCost)) {
            EditOrderFailureEvent failureEvent(event.getBrokerId(), parentOrder.symbolId_, parentOrder.side_, parentOrder.price_, parentOrder.orderVolume_, parentOrder.filledVolume_,
             EditOrderFailureEvent::FailureReason::INSUFFICIENT_FUNDS, event.getNewPrice(), event.getNewVolume());
            notifyOrderManagerEventListners(failureEvent);
            return 0;
        }
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
    symbolStates_[parentOrder.symbolId_].onSingleOrderEdit(brokerId_, parentOrder.side_, parentOrder.price_, event.getNewPrice(), parentOrder.orderVolume_, event.getNewVolume());
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    return order.orderId_;
}   

void  OrderManager::handleEvent(const CancelOrderRejectEvent& event)
{
    auto comOrder = getEditableOrderEntry(event.getOrderId());

    if (!comOrder)
        return;

    auto &cancelOrder = std::get<CancelOrder>(*comOrder);
    auto parentComOrder = getEditableOrderEntry(cancelOrder.parentOriderId_);

    if (!parentComOrder)
        return;

    auto &parentOrder = std::get<SingleOrder>(*parentComOrder);
    CancelOrderFailureEvent failureEvent(event.getBrokerId(), parentOrder.symbolId_, parentOrder.side_, parentOrder.price_, parentOrder.orderVolume_, parentOrder.filledVolume_, CancelOrderFailureEvent::FailureReason::BROKER_REJECTION);

    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    auto cancelledVolume = parentOrder.orderVolume_ - parentOrder.filledVolume_;
    pendingCancellations_[static_cast<std::size_t>(parentOrder.type_)] -= cancelledVolume;
    parentOrder.childOriderId_ = 0;
    auto entryIndex = orderIDVsEntryMap_[cancelOrder.orderId_];
    --freeEntryPtr_;
    freeEntries_[freeEntryPtr_] = entryIndex;
    symbolStates_[parentOrder.symbolId_].onCancelOrderReject(brokerId_, parentOrder.side_, parentOrder.price_, cancelledVolume);
    updateCount_.fetch_add(1, std::memory_order_acq_rel);

    notifyOrderManagerEventListners(failureEvent);
}

void OrderManager::handleEvent(const CancelOrderAckEvent& event)
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
    symbolStates_[parentOrder.symbolId_].onCancelOrderAck(brokerId_, parentOrder.side_, parentOrder.price_, cancelledVolume);
    updateCount_.fetch_add(1, std::memory_order_acq_rel);

    auto costForUnfilledVolume = brokerProfile_.calculateCost(parentOrder.price_, cancelledVolume, parentOrder.side_);
    brokerProfile_.creditBalance(costForUnfilledVolume);
}

void OrderManager::handleEvent(const EditOrderRejectEvent& event)
{
    auto comOrder = getEditableOrderEntry(event.getOrderId());

    if (!comOrder)
        return;

    auto &editOrder = std::get<EditOrder>(*comOrder);
    auto parentComOrder = getEditableOrderEntry(editOrder.parentOriderId_);

    if (!parentComOrder)
        return;

    auto &parentOrder = std::get<SingleOrder>(*parentComOrder);
    EditOrderFailureEvent failureEvent(event.getBrokerId(), parentOrder.symbolId_, parentOrder.side_, parentOrder.price_, parentOrder.orderVolume_, parentOrder.filledVolume_, EditOrderFailureEvent::FailureReason::BROKER_REJECTION, editOrder.price_, editOrder.orderVolume_);

    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    auto orderVolumeChange = editOrder.orderVolume_ - parentOrder.orderVolume_;
    pendingShares_[static_cast<std::size_t>(parentOrder.type_)] -= orderVolumeChange;
    parentOrder.childOriderId_ = 0;
    auto entryIndex = orderIDVsEntryMap_[editOrder.orderId_];
    --freeEntryPtr_;
    freeEntries_[freeEntryPtr_] = entryIndex;
    symbolStates_[parentOrder.symbolId_].onEditOrderReject(brokerId_, parentOrder.side_, parentOrder.price_, editOrder.price_, parentOrder.orderVolume_, editOrder.orderVolume_);
    updateCount_.fetch_add(1, std::memory_order_acq_rel);


    auto unfilledVolume = parentOrder.orderVolume_ - parentOrder.filledVolume_;
    auto costForUnfilledVolume = brokerProfile_.calculateCost(parentOrder.price_, unfilledVolume, parentOrder.side_);
    auto newUnfilledVolume = editOrder.orderVolume_ - parentOrder.filledVolume_;
    auto costForNewUnfilledVolume = brokerProfile_.calculateCost(editOrder.price_, newUnfilledVolume, parentOrder.side_);

    if (costForNewUnfilledVolume > costForUnfilledVolume) {

        auto additionalCost = costForNewUnfilledVolume - costForUnfilledVolume;
        brokerProfile_.creditBalance(additionalCost);
    }

    notifyOrderManagerEventListners(failureEvent);
}

void OrderManager::handleEvent(const EditOrderAckEvent& event)
{
    auto comOrder = getEditableOrderEntry(event.getOrderId());

    if (!comOrder)
        return;

    auto &editOrder = std::get<EditOrder>(*comOrder);
    auto parentComOrder = getEditableOrderEntry(editOrder.parentOriderId_);

    if (!parentComOrder)
        return;

    auto &parentOrder = std::get<SingleOrder>(*parentComOrder);
    SingleOrder newOrder(event.getBrokerId(), parentOrder.side_, event.getOrderId(), editOrder.price_, editOrder.orderVolume_);
    newOrder.filledVolume_ = parentOrder.filledVolume_;

    updateCount_.fetch_add(1, std::memory_order_acq_rel);

    auto entryIndex = orderIDVsEntryMap_[parentOrder.orderId_];
    --freeEntryPtr_;
    freeEntries_[freeEntryPtr_] = entryIndex;

    orderIDVsEntryMap_[event.getOrderId()] = newOrder.orderId_;
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
}