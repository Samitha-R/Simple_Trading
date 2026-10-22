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

    OrderEntry newOrder(currentOrderID_, event.getSide(), event.getSymbolId(), event.getPrice(), event.getVolume());
    auto freeEntryIndex = freeEntries_[freeEntryPtr_];
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    orderIDVsEntryMap_[currentOrderID_] = freeEntryIndex;
    orderEntries_[freeEntryIndex] = newOrder;
    pendingShares_[static_cast<std::size_t>(event.getSide())] += event.getVolume();
    pendingShareValues_[static_cast<std::size_t>(event.getSide())] += event.getVolume() * event.getPrice();
    ++freeEntryPtr_;
    ++currentOrderID_;
    symbolStates_[event.getSymbolId()].onSingleOrder(event.getBrokerId(), event.getSide(), event.getPrice(), event.getVolume());
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    return newOrder.getOrderId();
}

bool OrderManager::handleEvent(const SingleOrderAckEvent& event)
{
    auto order = getEditableOrderEntry(event.getOrderId());

    if (!order)
        return false;

    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    order->setStatus(OrderState::NEW);
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    return true;
}

bool OrderManager::handleEvent(const SingleOrderRejectEvent& event)
{
    auto order = getEditableOrderEntry(event.getOrderId());

    if (!order)
        return false;

    //auto &order = std::get<SingleOrder>(*comOrder);
    SingleOrderFailureEvent failureEvent(event.getBrokerId(), order->getSymbolId(), order->getSide(), order->getPrice(), order->getOrderVolume(),
    order->getFilledVolume(), SingleOrderFailureEvent::FailureReason::BROKER_REJECTION);

    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    order->setStatus(OrderState::REJECTED);
    auto unfilledVolume = order->getOrderVolume() - order->getFilledVolume();
    pendingShares_[static_cast<std::size_t>(order->getSide())] -= unfilledVolume;
    pendingShareValues_[static_cast<std::size_t>(order->getSide())] -= unfilledVolume * order->getPrice();
    auto entryIndex = orderIDVsEntryMap_[order->getOrderId()];
    --freeEntryPtr_;
    freeEntries_[freeEntryPtr_] = entryIndex;
    orderIDVsEntryMap_[order->getOrderId()] = orderEntryEnd_; // remove entry for rejected order
    symbolStates_[order->getSymbolId()].onSingleOrderReject(event.getBrokerId(), order->getSide(), order->getPrice(), unfilledVolume);
    updateCount_.fetch_add(1, std::memory_order_acq_rel);

    auto creditAmount = brokerProfile_.calculateCost(order->getPrice(), unfilledVolume, order->getSide());
    brokerProfile_.creditBalance(creditAmount);

    notifyOrderManagerEventListners(failureEvent);
    return true;
}

bool OrderManager::handleEvent(const OrderFillEvent& event)
{
    auto order = getEditableOrderEntry(event.getOrderId());

    if (!order)
        return false;

    auto comCountLocal = order->getFilledVolume()+ event.getFilledVolume();

    if ((event.getComVolume() != comCountLocal) || (event.getRemainingVolume() + event.getComVolume() != order->getOrderVolume()))
        return false;

    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    order->setFilledVolume(comCountLocal);
    order->setStatus(OrderState::PARTIALLY_FILLED);

    if (order->getSide() == OrderSide::BUY) {
        ownShares_ += event.getFilledVolume();
    } else {
        ownShares_ -= event.getFilledVolume();
    }

    pendingShares_[static_cast<std::size_t>(order->getSide())] -= event.getFilledVolume();
    pendingShareValues_[static_cast<std::size_t>(order->getSide())] -= event.getFilledVolume() * order->getPrice();
    symbolStates_[order->getSymbolId()].onOrderFill(event.getBrokerId(), order->getSide(), order->getPrice(), event.getFilledVolume() );

    if (order->getFilledVolume() == order->getOrderVolume()) {
        order->setStatus(OrderState::FILLED);

        auto entryIndex = orderIDVsEntryMap_[order->getOrderId()];
        --freeEntryPtr_;
        freeEntries_[freeEntryPtr_] = entryIndex;
        orderIDVsEntryMap_[order->getOrderId()] = orderEntryEnd_;
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
    order->addStatus(OrderState::PENDING_CANCEL);
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

    //EditOrder order(currentOrderID_, event.getOrderId(), event.getNewPrice(), event.getNewVolume());
    auto freeEntryIndex = freeEntries_[freeEntryPtr_];
    auto orderVolumeChange = event.getNewVolume() - order->getOrderVolume();

    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    order->addStatus(OrderState::PENDING_REPLACE);
    order->setNewPrice(event.getNewPrice());
    order->setNewVolume(event.getNewVolume());
    pendingShares_[static_cast<std::size_t>(order->getSide())] += orderVolumeChange;
    orderIDVsEntryMap_[currentOrderID_] = orderIDVsEntryMap_[event.getOrderId()];
    ++currentOrderID_;
    symbolStates_[order->getSymbolId()].onSingleOrderEdit(brokerProfile_.getID(), order->getSide(), order->getPrice(), event.getNewPrice(), order->getOrderVolume(), event.getNewVolume());
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    return currentOrderID_ - 1;
}   

bool  OrderManager::handleEvent(const CancelOrderRejectEvent& event)
{
    auto order = getEditableOrderEntry(event.getOrderId());  // as cancel order id  points to the parent order entry

    if (!order)
        return false;

    CancelOrderFailureEvent failureEvent(event.getBrokerId(), order->getSymbolId(), order->getSide(), order->getPrice(), order->getOrderVolume(), order->getFilledVolume(), CancelOrderFailureEvent::FailureReason::BROKER_REJECTION);
    auto cancelledVolume = order->getOrderVolume() - order->getFilledVolume();
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    order->removeStatus(OrderState::PENDING_CANCEL);
    pendingCancellations_[static_cast<std::size_t>(order->getSide())] -= cancelledVolume;
    symbolStates_[order->getSymbolId()].onCancelOrderReject(brokerProfile_.getID(), order->getSide(), order->getPrice(), cancelledVolume);
    orderIDVsEntryMap_[event.getOrderId()] = orderEntryEnd_; // remove entry for cancel order
    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    notifyOrderManagerEventListners(failureEvent);
    return true;
}

bool OrderManager::handleEvent(const CancelOrderAckEvent& event)
{
    auto order = getEditableOrderEntry(event.getOrderId());  // as cancel order id  points to the parent order entry

    if (!order)
        return false;

    auto cancelledVolume = order->getOrderVolume() - order->getFilledVolume();
    updateCount_.fetch_add(1, std::memory_order_acq_rel);

    pendingCancellations_[static_cast<std::size_t>(order->getSide())] -= cancelledVolume;
    pendingShares_[static_cast<std::size_t>(order->getSide())] -= cancelledVolume;
    auto entryIndex = orderIDVsEntryMap_[event.getOrderId()];
    --freeEntryPtr_;
    freeEntries_[freeEntryPtr_] = entryIndex;
    orderIDVsEntryMap_[order->getOrderId()] = orderEntryEnd_; // remove entry for order
    orderIDVsEntryMap_[event.getOrderId()] = orderEntryEnd_; // remove entry for cancel order
    symbolStates_[order->getSymbolId()].onCancelOrderAck(brokerProfile_.getID(), order->getSide(), order->getPrice(), cancelledVolume);
    updateCount_.fetch_add(1, std::memory_order_acq_rel);

    auto costForUnfilledVolume = brokerProfile_.calculateCost(order->getPrice(), cancelledVolume, order->getSide());
    brokerProfile_.creditBalance(costForUnfilledVolume);
    return true;
}

bool OrderManager::handleEvent(const EditOrderRejectEvent& event)
{
    auto order = getEditableOrderEntry(event.getOrderId()); // as edit order id  points to the parent order entry

    if (!order)
        return false;


    EditOrderFailureEvent failureEvent(event.getBrokerId(), order->getSymbolId(), order->getSide(), order->getPrice(), order->getOrderVolume(), order->getFilledVolume(), EditOrderFailureEvent::FailureReason::BROKER_REJECTION, order->getNewVolume(), order->getNewPrice());

    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    auto orderVolumeChange = order->getNewVolume() - order->getOrderVolume();
    pendingShares_[static_cast<std::size_t>(order->getSide())] -= orderVolumeChange;
    order->removeStatus(OrderState::PENDING_REPLACE);
    orderIDVsEntryMap_[event.getOrderId()] = orderEntryEnd_; // remove entry for edit order
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

    notifyOrderManagerEventListners(failureEvent);
    return true;
}

bool OrderManager::handleEvent(const EditOrderAckEvent& event)
{
    auto order = getEditableOrderEntry(event.getOrderId()); // as edit order id  points to the parent order entry

    if (!order)
        return false;


    updateCount_.fetch_add(1, std::memory_order_acq_rel);
    order->setVolume(order->getNewVolume());
    order->setPrice(order->getNewPrice());
    order->removeStatus(OrderState::PENDING_REPLACE);
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