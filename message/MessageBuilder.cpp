#include "MessageBuilder.h"
#include <cstring>
#include <chrono>

const char* OutMessage::getDataToBeSent(size_t& length) const
{
    length = dataSize_ - sentCount_;
    return buffer_.data() + sentCount_;
}

void OutMessage::reset()
{
    dataSize_ = 0;
    sentCount_ = 0;
    status_ = MessageStatus::INITIAL;
}

std::ostream& operator<<(std::ostream& os, const OutMessage& message)
{
    for (std::size_t i = 0; i < message.dataSize_; ++i) {
        if (message.buffer_[i] == SOH)
            std::cout << message.buffer_[i] << "|";
        else
            std::cout << message.buffer_[i];
    }
    std::cout << std::endl;
    return os;
}

MessageBuilder::MessageBuilder(const TradeSymbols& symbols, std::size_t bodyLength, std::size_t maxSeqNum, TimeStampAccuracy timeAccuracy) : timeAccuracy_(timeAccuracy), symbols_(symbols)
{
    auto numDigits = [](std::size_t n)->std::size_t {
        std::size_t ans = 1;

        while (( n = n / 10))  {
            ++ans;
        }
        return ans;
    };

    bodyLengthPlaceHolder_ = std::string(numDigits(bodyLength), '0');
    seqNumPlaceHolder_ = std::string(numDigits(maxSeqNum), '0');
    timeStampPlaceHolder_ = std::string(18 + static_cast<int>(timeAccuracy), '0');
}

bool MessageBuilder::addDataToOutMsg(const FixLogonMessage& msg, OutMessage& outMessage, std::string_view id, std::string_view targetId)
{
    return addDataToOutMsgTmp(msg, outMessage, id, targetId);

}

bool MessageBuilder::addDataToOutMsg(const FixHeartBeatMessage& msg, OutMessage& outMessage, std::string_view id, std::string_view targetId)
{
    return addDataToOutMsgTmp(msg, outMessage, id, targetId);
}

bool MessageBuilder::addDataToOutMsg(const FixMarketDataRequest& msg, OutMessage& outMessage, std::string_view id, std::string_view targetId)
{
    return addDataToOutMsgTmp(msg, outMessage, id, targetId);
}

bool MessageBuilder::addDataToOutMsg(const FixLogonMessage& msg, OutMessage& outMessage)
{
    auto userName = msg.getUserName();
    auto passWord = msg.getPassWord();

    if (!userName.empty() && !passWord.empty()) {
        auto st = addTagValue(outMessage, Username::name_, Username::length_, userName.data(), userName.length());

        if (st != MessageBuilder::UpdateStatus::SUCCESS)
            return false;

        st = addTagValue(outMessage, Password::name_, Password::length_, passWord.data(), passWord.length());

        if (st != MessageBuilder::UpdateStatus::SUCCESS)
            return false;
    
    }

    auto st = addTagValue(outMessage, ResetSeqNumFlag::name_, ResetSeqNumFlag::length_, static_cast<char>(ResetSeqNumFlag::Types::YES));

    if (st != MessageBuilder::UpdateStatus::SUCCESS)
        return false;

    st = addTagValue(outMessage, EncryptMethod::name_, EncryptMethod::length_, msg.getEncryptMethod());

    if (st != MessageBuilder::UpdateStatus::SUCCESS)
        return false;

    st = addTagValue(outMessage, HeartBtInt::name_, HeartBtInt::length_, msg.getHeartBeatInterval());

    if (st != MessageBuilder::UpdateStatus::SUCCESS)
        return false;

    return true;
}

bool  MessageBuilder::addDataToOutMsg(const FixHeartBeatMessage& msg, OutMessage& outMessage)
{
    auto testIdLength = msg.getTestIdLength();

    if (testIdLength > 0) {
        auto st = addTagValue(outMessage, TestReqID::name_, TestReqID::length_, msg.getTestID(), testIdLength);

        if (st != MessageBuilder::UpdateStatus::SUCCESS)
            return false;
    }

    return true;
}

bool MessageBuilder::addDataToOutMsg(const FixMarketDataRequest& msg, OutMessage& outMessage)
{
    if (!msg.getReqID() || msg.getSymbols().empty() || (msg.getSubscriptionType() == SubscriptionRequestType::Types::UNDEFINED) ||
        (msg.getUpdateType() ==  UpdateType::Types::UNDEFINED)) {
            return false;
    }

    auto st = addTagValue(outMessage, ReqID::name_, ReqID::length_, msg.getReqID());

    if (st != MessageBuilder::UpdateStatus::SUCCESS)
        return false;

    st = addTagValue(outMessage, SubscriptionRequestType::name_, SubscriptionRequestType::length_, static_cast<std::size_t>(!msg.getReqID()));

    if (st != MessageBuilder::UpdateStatus::SUCCESS)
        return false;

    st = addTagValue(outMessage, MarketDepth::name_, MarketDepth::length_, msg.getMarketDepth());

    if (st != MessageBuilder::UpdateStatus::SUCCESS)
        return false;

    st = addTagValue(outMessage, UpdateType::name_, UpdateType::length_, static_cast<std::size_t>(msg.getUpdateType()));

    if (st != MessageBuilder::UpdateStatus::SUCCESS)
        return false;

    st = addTagValue(outMessage, NoRelatedSym::name_, NoRelatedSym::length_, msg.getSymbols().size());

    if (st != MessageBuilder::UpdateStatus::SUCCESS)
        return false;

    const auto& allSymbols = getSymbols();

    for (auto id : msg.getSymbols()) {
        auto symbolName = allSymbols.getSymbolName(id);
        st = addTagValue(outMessage, Symbol::name_, Symbol::length_, symbolName.data(), symbolName.length());

        if (st != MessageBuilder::UpdateStatus::SUCCESS)
            return false;
    }

    st = addTagValue(outMessage, NoMDEntryTypes::name_, NoMDEntryTypes::length_, msg.getEntryTypes().size());

    for (auto type : msg.getEntryTypes()) {
        st = addTagValue(outMessage, EntryType::name_, EntryType::length_, static_cast<char>(type));

        if (st != MessageBuilder::UpdateStatus::SUCCESS)
            return false;
    
    }

    return true;
}

bool MessageBuilder::finalizeOutMessage(OutMessage& outMessage, std::size_t seqNum)
{
    char seqNumStrTmp[seqNumPlaceHolder_.length()];

    auto st = std::to_chars(seqNumStrTmp, seqNumStrTmp + seqNumPlaceHolder_.length() -1, seqNum);

    if (st.ec != std::errc())
        return false;

    auto numLength = st.ptr - seqNumStrTmp;
    auto numStartPtr = outMessage.buffer_.data() + outMessage.seqNumPos_ + MsgSeqNum::length_ + 1 + seqNumPlaceHolder_.length() - numLength;

    std::memcpy(numStartPtr, seqNumStrTmp, numLength);

    auto now = std::chrono::system_clock::now();
    auto now_time = std::chrono::system_clock::to_time_t(now);
    std::tm utc_tm;
    gmtime_r(&now_time, &utc_tm);

    auto timestartPtr = outMessage.buffer_.data() + outMessage.timeStampPos_ + SendingTime::length_ + 1;
    auto timeEndPtr= timestartPtr + timeStampPlaceHolder_.length();

    st = std::to_chars(timestartPtr, timeEndPtr, utc_tm.tm_year + 1900);

    if (utc_tm.tm_mon + 1 < 10) {
       st = std::to_chars(st.ptr + 1, timeEndPtr, utc_tm.tm_mon + 1); 
    } else {
        st = std::to_chars(st.ptr, timeEndPtr, utc_tm.tm_mon + 1);
    }

    if (utc_tm.tm_mday < 10 ) {
        st = std::to_chars(st.ptr + 1, timeEndPtr, utc_tm.tm_mday); 
    } else {
        st = std::to_chars(st.ptr, timeEndPtr, utc_tm.tm_mday); 
    }

    *st.ptr = '-';
    ++st.ptr;

    if (utc_tm.tm_hour < 10 ) {
        st = std::to_chars(st.ptr + 1, timeEndPtr, utc_tm.tm_hour); 
    } else {
        st = std::to_chars(st.ptr, timeEndPtr, utc_tm.tm_hour); 
    }

    *st.ptr = ':';
    ++st.ptr;

    if (utc_tm.tm_min < 10 ) {
        st = std::to_chars(st.ptr + 1, timeEndPtr, utc_tm.tm_min); 
    } else {
        st = std::to_chars(st.ptr, timeEndPtr, utc_tm.tm_min); 
    }

    *st.ptr = ':';
    ++st.ptr;

    if (utc_tm.tm_sec < 10 ) {
        st = std::to_chars(st.ptr + 1, timeEndPtr, utc_tm.tm_sec); 
    } else {
        st = std::to_chars(st.ptr, timeEndPtr, utc_tm.tm_sec); 
    }

    *st.ptr = '.';
    ++st.ptr;

    long accuracy;
    if (timeAccuracy_ == TimeStampAccuracy::MILLI) {
        auto milli = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        accuracy = milli.count();
    } else if (timeAccuracy_ == TimeStampAccuracy::MICRO) {
        auto micro = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()) % 1000000;
        accuracy = micro.count();
    } else {
        auto nano = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()) % 1000000000;
        accuracy = nano.count();
    }

    char accuracyStr[10];
    st = std::to_chars(accuracyStr, accuracyStr + 9, accuracy);
    auto accuracyLength = st.ptr -accuracyStr;
    memcpy(timeEndPtr - accuracyLength, accuracyStr, accuracyLength);

    int checksum = 0;
    
    for (std::size_t i = 0; i <  outMessage.dataSize_; ++i) {
        checksum += static_cast<unsigned char>(outMessage.buffer_[i]);
    }

    checksum = checksum % 256;

    auto retVal = addTagValue(outMessage, CheckSum::name_, CheckSum::length_, checksum);

    if (retVal != UpdateStatus::SUCCESS)
        return false;

    outMessage.status_ = OutMessage::MessageStatus::FINALIZED;
    return true;
}

MessageBuilder::UpdateStatus MessageBuilder::addTagValue(OutMessage& msg, const char* tag, std::size_t tagLength, const char* value , std::size_t valueLength)
{
    if (msg.buffer_.size() < msg.dataSize_ + tagLength + valueLength + 2) {
        msg.status_ = OutMessage::MessageStatus::INVALID;
        return UpdateStatus::OVERFLOW;
    }

    memcpy(msg.buffer_.data() + msg.dataSize_, tag, tagLength);
    msg.dataSize_ += tagLength;
    msg.buffer_[msg.dataSize_] = '=';
    ++msg.dataSize_ ;
    memcpy(msg.buffer_.data() + msg.dataSize_, value, valueLength);

    msg.dataSize_ += valueLength;
    msg.buffer_[msg.dataSize_] = SOH;
    ++msg.dataSize_;
    return UpdateStatus::SUCCESS;
}

MessageBuilder::UpdateStatus MessageBuilder::addTagValue(OutMessage& msg, const char* tag, std::size_t tagLength, char value)
{
    if (msg.buffer_.size() < msg.dataSize_ + tagLength + 3) {
        msg.status_ = OutMessage::MessageStatus::INVALID;
        return UpdateStatus::OVERFLOW;
    }

    memcpy(msg.buffer_.data() + msg.dataSize_, tag, tagLength);
    msg.dataSize_ += tagLength;
    msg.buffer_[msg.dataSize_] = '=';
    ++msg.dataSize_ ;
     msg.buffer_[msg.dataSize_] = value;
    ++msg.dataSize_ ;
    msg.buffer_[msg.dataSize_] = SOH;
    ++msg.dataSize_;
    return UpdateStatus::SUCCESS;
}

