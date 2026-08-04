#ifndef EVENTS_H
#define EVENTS_H

enum class EventType {
    MARKET_CHANGE,
    NEW_FIX_MESSAGE,
    BROKER_CONNECTION_CLOSED,
    LOGON_SUCCESS,
    SINGLE_ORDER,
    SINGLE_ORDER_ACK,
    SINGLE_ORDER_REJECT,
    ORDER_FILL,
    CANCEL_ORDER,
    CANCEL_ORDER_ACK,
    CANCEL_ORDER_REJECT,
    EDIT_ORDER,
    EDIT_ORDER_ACK,
    EDIT_ORDER_REJECT,

    SINGLE_ORDER_FAILURE,
    CANCEL_ORDER_FAILURE,
    EDIT_ORDER_FAILURE
};

class EventBase {
public:
    EventBase(EventType type) : type_(type) { }
    EventType getEventType() const { return type_; }
private:
    EventType type_;
};


using notifyFuncType = void(*)(void*, const EventBase&);

template <typename T> void notify(void* obj, const EventBase& event)
{
    T* subscriber = static_cast<T*>(obj);
    subscriber->notify(event);
}

class Subscriber
{
public:
    template<typename T> Subscriber(T* object, notifyFuncType func) : object_(object), func_(func) {}
    void notify(const EventBase& event) { func_(object_, event); }
private:
    void *object_;
    notifyFuncType func_;
};

#endif