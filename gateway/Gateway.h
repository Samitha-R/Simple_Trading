#ifndef GATEWAY_H
#define GATEWAY_H

#include <thread>
#include <vector>
#include <exception>
#include "SessionWrapper.h"

template<template <typename, typename, typename> class SessionType, typename SessionWrapper> class Gateway
{
public:
    Gateway(const Gateway&) = delete;
    Gateway& operator=(const Gateway&) = delete;
    Gateway(const Gateway&&) = delete;
    Gateway& operator=(const Gateway&&) = delete;
public:
    void exec(const std::vector<SessionWrapper*>& sessionList);
    Gateway(std::size_t numBrokers, std::size_t numThreads);
    template<typename... Args> BrokerID addSession(std::size_t threadId, Args&&... arg);
    template<typename... Args> BrokerID addSessionForMainThread(Args&&... arg);
    bool start();
    void stop();
protected:
    std::vector<SessionWrapper> brokers_;
    std::vector<std::jthread> threads_;
    std::vector<std::vector<SessionWrapper*>> threadVsSessoins_;
    std::vector<SessionWrapper*> mainThreadSesions_;
    bool useMainThread_ = false;
    bool running_ = false;
};

template<template<typename, typename, typename> class SessionType, typename SessionWrapper> Gateway<SessionType, SessionWrapper>::Gateway(std::size_t numBrokers, std::size_t numThreads) : threads_(numThreads), threadVsSessoins_(numThreads)
{
    brokers_.reserve(numBrokers);
    mainThreadSesions_.reserve(1);
}

template<template<typename, typename, typename> class SessionType, typename SessionWrapper> template<typename... Args> BrokerID Gateway<SessionType, SessionWrapper>::addSession(std::size_t threadId, Args&&... arg)
{
    if (threadId >= threads_.size()) {
        return NoBrokerID;
    }

    BrokerID id = brokers_.size();
    auto feedHandler =  std::unique_ptr(new SessionType(std::forward<Args>(arg)...)) ;
    feedHandler->setBrokerID(id);
    brokers_.push_back(SessionWrapper(std::move(feedHandler)));
    threadVsSessoins_[threadId].push_back(&(brokers_[id]));
    return id;
}

template<template<typename, typename, typename> class SessionType, typename SessionWrapper> template<typename... Args> BrokerID Gateway<SessionType, SessionWrapper>::addSessionForMainThread(Args&&... arg)
{
    BrokerID id = brokers_.size();
    auto feedHandlerPtr = new SessionType(std::forward<Args>(arg)...);
    auto uniPtr = std::unique_ptr<std::remove_pointer_t<decltype(feedHandlerPtr)>>(feedHandlerPtr);
    feedHandlerPtr->setBrokerID(id);
    brokers_.push_back(SessionWrapper(std::move(uniPtr)));
    mainThreadSesions_.push_back(&(brokers_[id]));
    return id;
}

template<template<typename MsgParser, typename MsgBuilder, typename Logger> class SessionType, typename SessionWrapper> bool Gateway<SessionType, SessionWrapper>::start()
{
    for (auto &broker : brokers_) {

        if (!broker.openSession())
            return false;
    }

    running_ = true;


    for (std::size_t i = 0; i < threads_.size(); ++i) {
        threads_[i] = std::jthread(&Gateway<SessionType, SessionWrapper>::exec, this, threadVsSessoins_[i]);
    }

    if (mainThreadSesions_.size()) {
        exec(mainThreadSesions_);
    }

    return true;
}

template<template<typename MsgParser, typename MsgBuilder, typename Logger> class SessionType, typename SessionWrapper> void Gateway<SessionType, SessionWrapper>::exec(const std::vector<SessionWrapper*>& sessionList)
{
    if (sessionList.empty())
        return;

    while(running_) {

        for (auto &session : sessionList) {

            if (session->isSessionReady()) {
                session->sendMessages();
                session->readMessages();
                session->checkAndSendHeartBeat();
            }
            
        }
    }
}

template<template<typename MsgParser, typename MsgBuilder, typename Logger> class SessionType, typename SessionWrapper> void Gateway<SessionType, SessionWrapper>::stop()
{
    running_ = false;

    for (auto &thread :threads_) {

        if (thread.joinable()) 
            thread.join();      
    }

}


#endif
