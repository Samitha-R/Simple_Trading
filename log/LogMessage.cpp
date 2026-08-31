#include "LogMessage.h"


SessionCreated::SessionCreated(SessionIDType id, PortType port, std::string_view hostName) : time_(getCurrentTimeStamp()), id_(id), port_(port)
{
    auto size = std::min(hostName.size(), sizeof(hostName_) - 1);
    std::memcpy(hostName_, hostName.data(), size);
    hostName_[size] = '\0';
}

void StringLog::setMessage(std::string_view msg) {
        msgSize_ = std::min(msg.size(), sizeof(message_));
        std::memcpy(message_, msg.data(), msgSize_);
}