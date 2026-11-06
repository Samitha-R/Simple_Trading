#ifndef SESSION_WRAPPER_H
#define SESSION_WRAPPER_H

#include <memory>
#include "Session.h"

class SessionWrapper
{
    friend void swap(SessionWrapper& l, SessionWrapper& r);
public:
    template<typename Session> SessionWrapper(std::unique_ptr<Session> session);
    SessionWrapper(const SessionWrapper&) = delete;
    SessionWrapper& operator=(const SessionWrapper&) = delete;
    SessionWrapper(SessionWrapper&&);
    SessionWrapper& operator=(SessionWrapper&&);
    ~SessionWrapper() { if (delete_) delete_(session_); }
    bool openSession() { return openSession_(session_); }
    SessionStatus checkStatus() { return checkStatus_(session_); }
    bool isSessionReady() { return isSessionReady_(session_);}
    void readMessages() { readMessages_(session_); }
    void sendMessages() { sendMessages_(session_); }
    void checkAndSendHeartBeat() { checkAndSendHeartBeat_(session_); }
    void registerForSessionEvents(Subscriber subscriber) { registerForSessionEvents_(session_, subscriber);}
    void registerForEvents(Subscriber subscriber) { registerForEvents_(session_, subscriber); }
protected:
    void* session_ = nullptr;
private:
    bool(*openSession_)(void*) = nullptr;
    SessionStatus(*checkStatus_)(void*) = nullptr;
    bool(*isSessionReady_)(void*) = nullptr;
    void(*readMessages_)(void*) = nullptr;
    void(*sendMessages_)(void*) = nullptr;
    void(*checkAndSendHeartBeat_)(void*) = nullptr;
    void(*registerForSessionEvents_)(void*, Subscriber) = nullptr;
    void(*registerForEvents_)(void*, Subscriber) = nullptr;
    void(*delete_)(void*) = nullptr;
};

template<typename Session> SessionWrapper::SessionWrapper(std::unique_ptr<Session> session) : session_(session.release())
{
    openSession_ = [](void* session)->auto{
        return  static_cast<Session*>(session)->openSession();
    };

    checkStatus_ = [](void* session)->auto {
        return static_cast<Session*>(session)->checkStatus();
    };

    isSessionReady_ = [](void* session)->auto {
        return static_cast<Session*>(session)->isSessionReady();
    };

    readMessages_ = [](void* session) {
        static_cast<Session*>(session)->readMessages();
    };

    sendMessages_ = [](void* session) {
        static_cast<Session*>(session)->sendMessages();
    };

    checkAndSendHeartBeat_ = [](void* session) {
        static_cast<Session*>(session)->checkAndSendHeartBeat();
    };

    registerForSessionEvents_= [](void* session, Subscriber subscriber) {
        static_cast<Session*>(session)->registerForSessionEvents(subscriber);
    };

    registerForEvents_= [](void* session, Subscriber subscriber) {
        static_cast<Session*>(session)->registerForEvents(subscriber);
    };

    delete_ = [](void* session) {
        delete static_cast<Session*>(session);
    };
}
#endif