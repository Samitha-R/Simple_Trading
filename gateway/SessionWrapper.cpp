#include "SessionWrapper.h"

void swap(SessionWrapper& l, SessionWrapper& r)
{
    std::swap(l.session_, r.session_);
    std::swap(l.sendLogon_, r. sendLogon_);
    std::swap(l.sendMDRquest_, r.sendMDRquest_);
    std::swap(l. openSession_, r. openSession_ );
    std::swap(l.checkStatus_, r.checkStatus_);
    std::swap(l.isSessionReady_, r.isSessionReady_);
    std::swap(l.readMessages_, r.readMessages_);
    std::swap(l.sendMessages_, r.sendMessages_);
    std::swap(l.checkAndSendHeartBeat_, r.checkAndSendHeartBeat_);
    std::swap(l.registerForSessionEvents_, r.registerForSessionEvents_);
    std::swap(l.registerForEvents_, r.registerForSessionEvents_);
    std::swap(l.getLogonMessage_, r.getLogonMessage_);
    std::swap(l.getHeartBeatMessage_, r.getHeartBeatMessage_);
    std::swap(l.getMarketDataRequest_, r.getMarketDataRequest_);
    std::swap(l.delete_, r.delete_);
}

SessionWrapper::SessionWrapper(SessionWrapper&& sw) :  session_(sw.session_), sendLogon_(sw.sendLogon_), sendMDRquest_(sw.sendMDRquest_),
                                                       openSession_(sw.openSession_), checkStatus_(sw.checkStatus_), isSessionReady_(sw.isSessionReady_),
                                                       readMessages_(sw.readMessages_), sendMessages_(sw.sendMessages_), checkAndSendHeartBeat_(sw.checkAndSendHeartBeat_),
                                                       registerForSessionEvents_(sw.registerForSessionEvents_),
                                                       registerForEvents_(sw.registerForEvents_), getLogonMessage_(sw.getLogonMessage_),
                                                       getHeartBeatMessage_(sw.getHeartBeatMessage_), getMarketDataRequest_(sw.getMarketDataRequest_),
                                                       delete_(sw.delete_)

{
    sw.session_ = nullptr;
    sw.sendLogon_ = nullptr;
    sw.sendMDRquest_ = nullptr;
    sw.openSession_ = nullptr;
    sw.checkStatus_ = nullptr;
    sw.isSessionReady_ = nullptr;
    sw.readMessages_ = nullptr;
    sw.sendMessages_ = nullptr;
    sw.checkAndSendHeartBeat_ = nullptr;
    sw.registerForSessionEvents_ = nullptr;
    sw.registerForEvents_ = nullptr;
    sw.getLogonMessage_ = nullptr;
    sw.getHeartBeatMessage_ = nullptr;
    sw.getMarketDataRequest_ = nullptr;
    sw.delete_ = nullptr;


}

SessionWrapper& SessionWrapper::operator=(SessionWrapper&& r)
{
    if (this == &r)
        return *this;

    swap(*this, r);
    return *this;

}
