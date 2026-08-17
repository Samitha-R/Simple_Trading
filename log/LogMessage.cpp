#include "LogMessage.h"


void ConnectionSuccess1::setHostName(std::string_view hostName) {
        auto size = std::min(hostName.size(), sizeof(hostName_) - 1);
        std::memcpy(hostName_, hostName.data(), size);
        hostName_[size] = '\0';
}

void StringMessage::setMessage(std::string_view msg) {
        msgSize_ = std::min(msg.size(), sizeof(message_));
        std::memcpy(message_, msg.data(), msgSize_);
}