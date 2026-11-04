#ifndef EVENTS_H
#define EVENTS_H

enum class EventType {
    MARKET_CHANGE,
    NEW_FIX_MESSAGE,
    BROKER_CONNECTION_CLOSED,
    LOGON_SUCCESS,
    SINGLE_ORDER,
    CANCEL_ORDER,
    EDIT_ORDER,
    SINGLE_ORDER_FAILURE,
    CANCEL_ORDER_FAILURE,
    EDIT_ORDER_FAILURE,
    EXECUTION_REPORT_EVENT,
    ORDER_CANCEL_REJECT_EVENT
};

class EventBase {
public:
    EventBase(EventType type) : type_(type) { }
    EventType getEventType() const { return type_; }
private:
    EventType type_;
};




class Subscriber
{
    using notifyFuncType = void(*)(void*, const EventBase&);
public:
    Subscriber() = default;
    template<typename T> Subscriber(T* object) : object_(object), 
    func_([] (void* object, const EventBase& event) { T* subscriber = static_cast<T*>(object); subscriber->notify(event); }) {}
    void notify(const EventBase& event) { func_(object_, event); }
private:
    void *object_ = nullptr;
    notifyFuncType func_ = nullptr;
};

#endif