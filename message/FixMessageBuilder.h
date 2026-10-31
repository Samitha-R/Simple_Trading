#ifndef FIX_MESSAGE_BUILDER_H
#define FIX_MESSAGE_BUILDER_H

#include <string>
#include "FixTags.h"
#include "Message.h"
#include "CommonUtils.h"
#include "OutMessage.h"

namespace Fix
{
enum class TimeStampAccuracy {MILLI = 3, MICRO = 6, NANO = 9};

class MessageBuilder
{
public:
    enum class UpdateStatus { SUCCESS, FORMAT_ERROR, OVERFLOW};

public:
    MessageBuilder(const TradeSymbols& symbols, std::size_t bodyLength = 2048, std::size_t maxSeqNum = 1000000000, TimeStampAccuracy timeAccuracy = TimeStampAccuracy::MICRO);
    bool addDataToOutMsg(const LogonMessage& msg, OutMessage& outMessage, std::string_view id, std::string_view targetId);
    bool addDataToOutMsg(const HeartBeatMessage& msg, OutMessage& outMessage, std::string_view id, std::string_view targetId);
    bool addDataToOutMsg(const MarketDataRequest& msg, OutMessage& outMessage, std::string_view id, std::string_view targetId);
    bool addDataToOutMsg(const NewOrderSingleMessage& msg, OutMessage& outMessage, std::string_view id, std::string_view targetId);
    bool addDataToOutMsg(const OrderReplaceMessage& msg, OutMessage& outMessage, std::string_view id, std::string_view targetId);
    bool finalizeOutMessage(OutMessage& outMessage, std::size_t seqNum);
    void printOutMessage(const OutMessage& outMessage, std::ostream& os);
private:
    UpdateStatus addTagValue(OutMessage& msg, const char* tag, std::size_t tagLength, const char* value, std::size_t valueLength);
    template<typename T> UpdateStatus addTagValue(OutMessage& msg, const char* tag, std::size_t tagLength, T value);
    UpdateStatus addTagValue(OutMessage& msg, const char* tag, std::size_t tagLength, char value);
    UpdateStatus addTagValue(OutMessage& msg, const char* tag, std::size_t tagLength, ::Price value);
    const auto& getSymbols() const { return symbols_; }
private:
    template<typename T> bool addDataToOutMsgTmp(const T& msg, OutMessage& outMessage, std::string_view id, std::string_view targetId);
    bool addDataToOutMsg(const LogonMessage& msg, OutMessage& outMessage);
    bool addDataToOutMsg(const HeartBeatMessage& msg, OutMessage& outMessage);
    bool addDataToOutMsg(const MarketDataRequest& msg, OutMessage& outMessage);
    bool addDataToOutMsg(const NewOrderSingleMessage& msg, OutMessage& outMessage);
    bool addDataToOutMsg(const OrderReplaceMessage& msg, OutMessage& outMessage);
private:
    TimeStampAccuracy timeAccuracy_;
    std::string seqNumPlaceHolder_;
    std::string bodyLengthPlaceHolder_;
    std::string timeStampPlaceHolder_;
    const TradeSymbols& symbols_;
    MessageProtocol protocol_ = MessageProtocol::FIX44;
};

template<typename T> bool MessageBuilder::addDataToOutMsgTmp(const T& msg, OutMessage &outMessage, std::string_view id, std::string_view targetId)
{
    auto versionS = toFixProtocolString(protocol_ );

    auto st = addTagValue(outMessage, BeginString::name_, BeginString::length_, versionS.data(), versionS.size());

    if (st != UpdateStatus::SUCCESS)
        return false;
   
    outMessage.msgLenPos_ = outMessage.dataSize_;

    st =  addTagValue(outMessage, BodyLength::name_, BodyLength::length_, bodyLengthPlaceHolder_.data(), bodyLengthPlaceHolder_.size());

    if (st != UpdateStatus::SUCCESS)
        return false;
      
    auto bodyStartPos = outMessage.dataSize_;

    st = addTagValue(outMessage, MsgType::name_, MsgType::length_, static_cast<char>(toFixType(msg.getMessageType())));

    if (st != UpdateStatus::SUCCESS)
        return false;

    st = addTagValue(outMessage, SenderCompID::name_, SenderCompID::length_, id.data(), id.size());

    if (st != UpdateStatus::SUCCESS)
        return false;

    st = addTagValue(outMessage, TargetCompID::name_, TargetCompID::length_, targetId.data(), targetId.size());

    if (st != UpdateStatus::SUCCESS)
        return false;

    outMessage.seqNumPos_ = outMessage.dataSize_;

    st = addTagValue(outMessage, MsgSeqNum::name_, MsgSeqNum::length_, seqNumPlaceHolder_.data(), seqNumPlaceHolder_.size());

     if (st != UpdateStatus::SUCCESS)
        return false;
    
    outMessage.timeStampPos_ = outMessage.dataSize_;

    st = addTagValue(outMessage, SendingTime::name_, SendingTime::length_, timeStampPlaceHolder_.data(), timeStampPlaceHolder_.size());

    if (st != UpdateStatus::SUCCESS)
        return false; 
    
   if (!addDataToOutMsg(msg, outMessage))
      return false;

   auto bodyLength = outMessage.dataSize_ - bodyStartPos;

   char bodyLengthStrTmp[bodyLengthPlaceHolder_.length()];

   auto  convSt= std::to_chars(bodyLengthStrTmp, bodyLengthStrTmp + bodyLengthPlaceHolder_.length() -1, bodyLength);

    if (convSt.ec != std::errc())
        return false;

    auto numDigits = convSt.ptr - bodyLengthStrTmp;
    auto lengthStartPtr = outMessage.buffer_.data() + outMessage.msgLenPos_ + BodyLength::length_ + 1 + bodyLengthPlaceHolder_.length() - numDigits;
    std::memcpy(lengthStartPtr, bodyLengthStrTmp, numDigits);
    return true;
}

template<typename T> MessageBuilder::UpdateStatus MessageBuilder::addTagValue(OutMessage& msg, const char* tag, std::size_t tagLength, T value)
{
    if (msg.buffer_.size() < msg.dataSize_ + tagLength + 1) {
        msg.status_ = OutMessage::MessageStatus::INVALID;
        return UpdateStatus::OVERFLOW;
    }

    memcpy(msg.buffer_.data() + msg.dataSize_, tag, tagLength);
    msg.dataSize_ += tagLength;
    msg.buffer_[msg.dataSize_] = '=';
    ++msg.dataSize_ ;
    auto st = std::to_chars(msg.buffer_.data() + msg.dataSize_, msg.buffer_.data() + msg.buffer_.size(), value);

    if (st.ec != std::errc() || st.ptr >= msg.buffer_.data() + msg.buffer_.size() -1 ) {
        msg.status_ = OutMessage::MessageStatus::INVALID;
        return UpdateStatus::OVERFLOW;
    }

    msg.dataSize_ = st.ptr - msg.buffer_.data();

    msg.buffer_[msg.dataSize_] = SOH;
    ++msg.dataSize_;
    return UpdateStatus::SUCCESS;
}
}
#endif