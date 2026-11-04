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

bool  OrderManager::handleEvent(const ExecutionReportEvent& event)
{
    auto& msg = event.getMessage();
    auto type = msg.getExecType();

    switch (type) {
        case ExecType::NEW:
            return handleSingleOrderAckEvent(msg);
        case ExecType::REJECTED:
            return handleSingleOrderRejectEvent(msg);
        case ExecType::CANCELED:
            return handleCanceledOrder(msg);
        case ExecType::REPLACED:
            return handleEditOrderAckEvent(msg);
        case ExecType::TRADE:
        case ExecType::FILLED:
            return handleOrderFillEvent(msg);
        default:
            return false;
    }
}

bool OrderManager::handleEvent(const OrderCancelRejectEvent& event)
{
    auto& msg = event.getMessage();
    CxlRejResponseTo type = msg.getResponseTo();

    switch (type) {
        case  CxlRejResponseTo::ORDER_CANCEL:
            return handleCancelOrderRejectEvent(msg);
        case CxlRejResponseTo::ORDER_REPLACE:
            return handleEditOrderRejectEvent(msg);
        default:
            return false;  
    }

}

std::size_t OrderManager::handleEvent(const SingleOrderEvent& event)
{
    if (currentOrderID_ > maxOrders_) {
        SingleOrderFailureEvent failureEvent(event.getBrokerId(), event.getSymbolID(), event.getSide(), event.getOrderPrice(), event.getOrderVolume(),
        0, SingleOrderFailureEvent::FailureReason::DAILY_LIMIT_EXCEEDED);
        notifyOrderManagerEventListners(failureEvent);
        return 0;
    }

    if (freeEntryPtr_ >= orderEntrySlots_) {
        SingleOrderFailureEvent failureEvent(event.getBrokerId(), event.getSymbolID(), event.getSide(), event.getOrderPrice(), event.getOrderVolume(),
        0, SingleOrderFailureEvent::FailureReason::NO_SPACE_TO_SAVE_ORDER);
        notifyOrderManagerEventListners(failureEvent);
        return 0;
    }

    auto cost = brokerProfile_.calculateCost(event.getOrderPrice(), event.getOrderVolume(), event.getSide());

    if (!brokerProfile_.debitBalance(cost)) {
        // Reject order due to insufficient funds
        SingleOrderFailureEvent failureEvent(event.getBrokerId(), event.getSymbolID(), event.getSide(), event.getOrderPrice(), event.getOrderVolume(),
                                             0, SingleOrderFailureEvent::FailureReason::INSUFFICIENT_FUNDS);
        notifyOrderManagerEventListners(failureEvent);
        return 0;
    }

    OrderEntry newOrder(currentOrderID_, event.getSide(), event.getSymbolID(), event.getOrderPrice(), event.getOrderVolume());
    auto freeEntryIndex = freeEntries_[freeEntryPtr_];
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    orderIDVsEntryMap_[currentOrderID_] = freeEntryIndex;
    orderEntries_[freeEntryIndex] = newOrder;
    pendingShares_[static_cast<std::size_t>(event.getSide())] += event.getOrderVolume();
    pendingShareValues_[static_cast<std::size_t>(event.getSide())] += event.getOrderVolume() * event.getOrderPrice();
    ++freeEntryPtr_;
    ++currentOrderID_;
    symbolStates_[event.getSymbolID()].onSingleOrder(event.getBrokerId(), event.getSide(), event.getOrderPrice(), event.getOrderVolume());
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    return newOrder.getOrderId();
}

bool OrderManager::handleSingleOrderAckEvent(const ExecutionReportMessage& msg)
{
    auto order = getEditableOrderEntry(msg.getClientOrderID());

    if (!order)
        return false;

    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    order->setStatus(OrderStatus::NEW);
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    return true;       
}

bool OrderManager::handleSingleOrderRejectEvent(const ExecutionReportMessage& msg)
{
    auto clientOrderID = msg.getClientOrderID();

    auto order = getEditableOrderEntry(clientOrderID);

    if (!order)
        return false;

    SingleOrderFailureEvent failureEvent(brokerProfile_.getID(), order->getSymbolId(), order->getSide(), order->getPrice(), order->getOrderVolume(),
    order->getFilledVolume(), SingleOrderFailureEvent::FailureReason::BROKER_REJECTION);

    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    order->setStatus(OrderStatus::REJECTED);
    auto unfilledVolume = order->getOrderVolume() - order->getFilledVolume();
    pendingShares_[static_cast<std::size_t>(order->getSide())] -= unfilledVolume;
    pendingShareValues_[static_cast<std::size_t>(order->getSide())] -= unfilledVolume * order->getPrice();
    auto entryIndex = orderIDVsEntryMap_[order->getOrderId()];
    --freeEntryPtr_;
    freeEntries_[freeEntryPtr_] = entryIndex;
    orderIDVsEntryMap_[order->getOrderId()] = orderEntryEnd_; // remove entry for rejected order
    symbolStates_[order->getSymbolId()].onSingleOrderReject(brokerProfile_.getID(), order->getSide(), order->getPrice(), unfilledVolume);
    updateCount_.fetch_add(1, std::memory_order_acq_rel);

    auto creditAmount = brokerProfile_.calculateCost(order->getPrice(), unfilledVolume, order->getSide());
    brokerProfile_.creditBalance(creditAmount);

    notifyOrderManagerEventListners(failureEvent);
    return true;
}

bool OrderManager::handleOrderFillEvent(const ExecutionReportMessage& msg)
{
    auto orderID = msg.getClientOrderID();
    auto order = getEditableOrderEntry(orderID);

    if (!order)
        return false;

    auto lastFilledVolume = msg.getLastQuantity();
    auto comCountLocal = order->getFilledVolume( ) + lastFilledVolume;

    if ((msg.getCumVolume() != comCountLocal))
        return false;

    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    order->setFilledVolume(comCountLocal);
    order->setStatus(OrderStatus::PARTIALLY_FILLED);
    auto side = msg.getSide();

    if (side == OrderSide::BUY) {
        ownShares_ += lastFilledVolume;
    } else {
        ownShares_ -= lastFilledVolume;
    }

    auto lastPrice = msg.getLastPrice();
   
    pendingShares_[static_cast<std::size_t>(side)] -= lastFilledVolume;
    pendingShareValues_[static_cast<std::size_t>(side)] -= lastFilledVolume * lastPrice;
    symbolStates_[order->getSymbolId()].onOrderFill(brokerProfile_.getID(), side, lastPrice, lastFilledVolume);

    if (msg.getOrderStatus() ==  OrderStatus::FILLED) {
        order->setStatus(OrderStatus::FILLED);
        auto entryIndex = orderIDVsEntryMap_[orderID];
        --freeEntryPtr_;
        freeEntries_[freeEntryPtr_] = entryIndex;
        orderIDVsEntryMap_[orderID] = orderEntryEnd_;
    }

    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    
    return true;
}

std::size_t OrderManager::handleEvent(const CancelOrderEvent& event)
{
    auto order = getEditableOrderEntry(event.getOrderId());

    if (!order)
        return 0;

    if (currentOrderID_ > maxOrders_) {
        SingleOrderFailureEvent failureEvent(event.getBrokerId(), order->getSymbolId(), order->getSide(), order->getPrice(), order->getOrderVolume(),
        order->getFilledVolume(), SingleOrderFailureEvent::FailureReason::DAILY_LIMIT_EXCEEDED);
        notifyOrderManagerEventListners(failureEvent);
        return 0;
    }

    auto cancelVolume = order->getOrderVolume() - order->getFilledVolume();
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    pendingCancellations_[static_cast<std::size_t>(order->getSide())] += cancelVolume;
    orderIDVsEntryMap_[currentOrderID_] = orderIDVsEntryMap_[event.getOrderId()];
    order->addStatus(OrderStatus::PENDING_CANCEL);
    ++currentOrderID_;
    symbolStates_[order->getSymbolId()].onSingleOrderCancel(brokerProfile_.getID(), order->getSide(), order->getPrice(), cancelVolume);
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    return currentOrderID_ - 1;
}

std::size_t OrderManager::handleEvent(const EditOrderEvent& event)
{
    auto order = getEditableOrderEntry(event.getOrderId());

    if (!order)
        return 0;
    
    if (currentOrderID_ > maxOrders_) {
        EditOrderFailureEvent failureEvent(event.getBrokerId(), order->getSymbolId(), order->getSide(), order->getPrice(), order->getOrderVolume(), order->getFilledVolume(),
         EditOrderFailureEvent::FailureReason::DAILY_LIMIT_EXCEEDED, event.getNewVolume(), event.getNewPrice());
        notifyOrderManagerEventListners(failureEvent);
        return 0;
    }

    if (event.getNewVolume() < order->getFilledVolume()) {
        EditOrderFailureEvent failureEvent(event.getBrokerId(), order->getSymbolId(), order->getSide(), order->getPrice(), order->getOrderVolume(), order->getFilledVolume(),
         EditOrderFailureEvent::FailureReason::EDIT_VOLUME_LESS_THAN_FILLED_VOLUME, event.getNewVolume(), event.getNewPrice());
        notifyOrderManagerEventListners(failureEvent);
        return 0;
    }

    auto unfilledVolume = order->getOrderVolume() - order->getFilledVolume();
    auto costForUnfilledVolume = brokerProfile_.calculateCost(order->getPrice(), unfilledVolume, order->getSide());
    auto newUnfilledVolume = event.getNewVolume() - order->getFilledVolume();
    auto costForNewUnfilledVolume = brokerProfile_.calculateCost(event.getNewPrice(), newUnfilledVolume, order->getSide());

    if (costForNewUnfilledVolume > costForUnfilledVolume) {

        auto additionalCost = costForNewUnfilledVolume - costForUnfilledVolume;

        if (!brokerProfile_.debitBalance(additionalCost)) {
            EditOrderFailureEvent failureEvent(event.getBrokerId(), order->getSymbolId(), order->getSide(), order->getPrice(), order->getOrderVolume(), order->getFilledVolume(),
             EditOrderFailureEvent::FailureReason::INSUFFICIENT_FUNDS, event.getNewVolume(), event.getNewPrice());
            notifyOrderManagerEventListners(failureEvent);
            return 0;
        }
    } 

    auto freeEntryIndex = freeEntries_[freeEntryPtr_];
    auto orderVolumeChange = event.getNewVolume() - order->getOrderVolume();

    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    order->addStatus(OrderStatus::PENDING_REPLACE);
    order->setNewPrice(event.getNewPrice());
    order->setNewVolume(event.getNewVolume());
    pendingShares_[static_cast<std::size_t>(order->getSide())] += orderVolumeChange;
    orderIDVsEntryMap_[currentOrderID_] = orderIDVsEntryMap_[event.getOrderId()];
    ++currentOrderID_;
    symbolStates_[order->getSymbolId()].onSingleOrderEdit(brokerProfile_.getID(), order->getSide(), order->getPrice(), event.getNewPrice(), order->getOrderVolume(), event.getNewVolume());
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    return currentOrderID_ - 1;
}

bool OrderManager::handleCancelOrderRejectEvent(const OrderCancelRejectMessage& msg)
{
    auto clientOrderID = msg.getClientOrderID();

    auto order = getEditableOrderEntry(clientOrderID);  // as cancel order id  points to the parent order entry

    if (!order)
        return false;

    auto cancelledVolume = order->getOrderVolume() - order->getFilledVolume();
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    order->removeStatus(OrderStatus::PENDING_CANCEL);
    pendingCancellations_[static_cast<std::size_t>(order->getSide())] -= cancelledVolume;
    symbolStates_[order->getSymbolId()].onCancelOrderReject(brokerProfile_.getID(), order->getSide(), order->getPrice(), cancelledVolume);
    orderIDVsEntryMap_[clientOrderID] = orderEntryEnd_; // remove entry for cancel order
    updateCount_.fetch_add(1, std::memory_order_acq_rel);

    CancelOrderFailureEvent failureEvent(brokerProfile_.getID(), order->getSymbolId(), order->getSide(), order->getPrice(), order->getOrderVolume(), order->getFilledVolume(), CancelOrderFailureEvent::FailureReason::BROKER_REJECTION);
    notifyOrderManagerEventListners(failureEvent);
    return true;
}

bool OrderManager::handleCanceledOrder(const ExecutionReportMessage& msg)
{
    auto clientOrderID =  msg.getClientOrderID();
    auto order = getEditableOrderEntry(clientOrderID);  // as cancel order id  points to the parent order entry

    if (!order)
        return false;

    auto cancelledVolume = order->getOrderVolume() - order->getFilledVolume();
    updateCount_.fetch_add(1, std::memory_order_acq_rel);

    pendingCancellations_[static_cast<std::size_t>(order->getSide())] -= cancelledVolume;
    pendingShares_[static_cast<std::size_t>(order->getSide())] -= cancelledVolume;
    auto entryIndex = orderIDVsEntryMap_[clientOrderID];
    --freeEntryPtr_;
    freeEntries_[freeEntryPtr_] = entryIndex;
    orderIDVsEntryMap_[order->getOrderId()] = orderEntryEnd_; // remove entry for order
    orderIDVsEntryMap_[clientOrderID] = orderEntryEnd_; // remove entry for cancel order
    symbolStates_[order->getSymbolId()].onCancelOrderAck(brokerProfile_.getID(), order->getSide(), order->getPrice(), cancelledVolume);
    updateCount_.fetch_add(1, std::memory_order_acq_rel);

    auto costForUnfilledVolume = brokerProfile_.calculateCost(order->getPrice(), cancelledVolume, order->getSide());
    brokerProfile_.creditBalance(costForUnfilledVolume);
    return true;
}

bool OrderManager::handleEditOrderRejectEvent(const OrderCancelRejectMessage& msg)
{
    auto clientOrderID = msg.getClientOrderID();
    auto order = getEditableOrderEntry(clientOrderID); // as edit order id  points to the parent order entry

    if (!order)
        return false;

    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    auto orderVolumeChange = order->getNewVolume() - order->getOrderVolume();
    pendingShares_[static_cast<std::size_t>(order->getSide())] -= orderVolumeChange;
    order->removeStatus(OrderStatus::PENDING_REPLACE);
    orderIDVsEntryMap_[clientOrderID] = orderEntryEnd_; // remove entry for edit order
    symbolStates_[order->getSymbolId()].onEditOrderReject(brokerProfile_.getID(), order->getSide(), order->getPrice(), order->getNewPrice(), order->getOrderVolume(), order->getNewVolume());
    updateCount_.fetch_add(1, std::memory_order_acq_rel);

    auto unfilledVolume = order->getOrderVolume() - order->getFilledVolume();
    auto costForUnfilledVolume = brokerProfile_.calculateCost(order->getPrice(), unfilledVolume, order->getSide());
    auto newUnfilledVolume = order->getNewVolume() - order->getFilledVolume();
    auto costForNewUnfilledVolume = brokerProfile_.calculateCost(order->getNewPrice(), newUnfilledVolume, order->getSide());

    if (costForNewUnfilledVolume > costForUnfilledVolume) {

        auto additionalCost = costForNewUnfilledVolume - costForUnfilledVolume;
        brokerProfile_.creditBalance(additionalCost);
    }

    EditOrderFailureEvent failureEvent(brokerProfile_.getID(), order->getSymbolId(), order->getSide(), order->getPrice(), order->getOrderVolume(), order->getFilledVolume(), EditOrderFailureEvent::FailureReason::BROKER_REJECTION, order->getNewVolume(), order->getNewPrice());
    notifyOrderManagerEventListners(failureEvent);
    return true;
}

bool OrderManager::handleEditOrderAckEvent(const ExecutionReportMessage& msg)
{
    auto clientOrderID = msg.getClientOrderID();
    auto order = getEditableOrderEntry(clientOrderID); // as edit order id  points to the parent order entry

    if (!order)
        return false;

    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    order->setVolume(order->getNewVolume());
    order->setPrice(order->getNewPrice());
    order->removeStatus(OrderStatus::PENDING_REPLACE);
    orderIDVsEntryMap_[order->getOrderId()] = orderEntryEnd_; // remove entry for original order
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    return true;
}


std::ostream& operator<<(std::ostream &os, const OrderEntry& order)
{
    os << "Order ID: " << order.getOrderId() << ", Side: " << (order.getSide() == OrderSide::BUY ? "BUY" : "SELL") 
       << ", Symbol ID: " << order.getSymbolId() << ", Price: " << order.getPrice() 
       << ", Volume: " << order.getOrderVolume() << ", Filled Volume: " << order.getFilledVolume()
       << ", New Price: " << order.getNewPrice() << ", New Volume: " << order.getNewVolume()
       << ", Status: " << order.getStatus();
    return os;
}

std::ostream& operator<<(std::ostream &os, const OrderManager& om)
{

    os << "ownShares=" << om.ownShares_ << ", pendingShares(BUY)=" << om.pendingShares_[0] << ",  pendingShares(SELL)=" << om.pendingShares_[1] << ", pendingCancellations(BUY)=" << om.pendingCancellations_[0] << ", pendingCancellations(SELL)=" << om.pendingCancellations_[1] << ", pendingShareValues(BUY)=" << om.pendingShareValues_[0] << ", pendingShareValues(SELL)=" << om.pendingShareValues_[1] << std::endl;

    for (auto i = 1; i < om.currentOrderID_; ++i) {
        auto entryIndex = om.orderIDVsEntryMap_[i];

        if (entryIndex == om.orderEntryEnd_)
            continue;

        const auto& order = om.orderEntries_[entryIndex];
        os << order << std::endl;
    }
    
    for (const auto& symbolState : om.symbolStates_) {
        os << symbolState << std::endl;
    }

    return os;
}