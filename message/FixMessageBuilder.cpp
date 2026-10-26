#include "FixMessageBuilder.h"
#include <cstring>
#include <chrono>

namespace Fix
{

std::size_t toFixFormat(std::chrono::system_clock::time_point time, char* bufferStart,  std::size_t bufferLength, TimeStampAccuracy timeAccuracy)
{
    auto requiredLength = 18 + static_cast<int>(timeAccuracy);

    if (bufferLength < 18 + static_cast<int>(timeAccuracy))
	    return 0;

    char *bufferEnd = bufferStart + requiredLength;
    auto dp = std::chrono::floor<std::chrono::days>(time);
    std::chrono::year_month_day ymd{dp};

    int year       = static_cast<int>(ymd.year());
    unsigned month = static_cast<unsigned>(ymd.month());
    unsigned day   = static_cast<unsigned>(ymd.day());
    

    std::size_t hour, minute, second, subsecond;

    auto dayAndTime = [&hour, &minute, &second, &subsecond]<typename T>(const T& time_of_day) {
        hour      = time_of_day.hours().count();
        minute    = time_of_day.minutes().count();
        second    = time_of_day.seconds().count();
        subsecond = time_of_day.subseconds().count();
    };

    if (timeAccuracy == TimeStampAccuracy::MILLI) {
        std::chrono::hh_mm_ss<std::chrono::milliseconds> time_of_day{duration_cast<std::chrono::milliseconds>(time - dp)};
        dayAndTime(time_of_day);
    } else if (timeAccuracy == TimeStampAccuracy::MICRO) {
        std::chrono::hh_mm_ss<std::chrono::microseconds> time_of_day{duration_cast<std::chrono::microseconds>(time - dp)};
        dayAndTime(time_of_day);
    } else {
            std::chrono::hh_mm_ss<std::chrono::nanoseconds> time_of_day{duration_cast<std::chrono::nanoseconds>(time - dp)};
        dayAndTime(time_of_day);
    }

    auto st = std::to_chars(bufferStart, bufferEnd, year);

    if (month < 10) {
	    *st.ptr = '0';
        ++st.ptr; 
    }

    st = std::to_chars(st.ptr, bufferEnd, month);

    if (day < 10 ) {
	    *st.ptr = '0';
        ++st.ptr;
    }

    st = std::to_chars(st.ptr, bufferEnd, day);

    *st.ptr = '-';
    ++st.ptr;

    if (hour < 10 ) {
	    *st.ptr = '0';
        ++st.ptr;
    }
    
    st = std::to_chars(st.ptr, bufferEnd, hour); 

    *st.ptr = ':';
    ++st.ptr;

    if (minute < 10 ) {
	    *st.ptr = '0';
        ++st.ptr;
    }

    st = std::to_chars(st.ptr, bufferEnd, minute);

    *st.ptr = ':';
    ++st.ptr;

    if (second < 10 ) {
	    *st.ptr = '0';
        ++st.ptr;
    }
    st = std::to_chars(st.ptr, bufferEnd, second);
     
    *st.ptr = '.';
    ++st.ptr;

    char accuracyStr[10];
    auto st2 = std::to_chars(accuracyStr, accuracyStr + 10, subsecond);
    auto accuracyLength = st2.ptr - accuracyStr;
    std::memset(st.ptr, '0', static_cast<int>(timeAccuracy) - accuracyLength);
    memcpy(bufferEnd - accuracyLength, accuracyStr, accuracyLength);
    return requiredLength;

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

bool MessageBuilder::addDataToOutMsg(const LogonMessage& msg, OutMessage& outMessage, std::string_view id, std::string_view targetId)
{
    return addDataToOutMsgTmp(msg, outMessage, id, targetId);

}

bool MessageBuilder::addDataToOutMsg(const HeartBeatMessage& msg, OutMessage& outMessage, std::string_view id, std::string_view targetId)
{
    return addDataToOutMsgTmp(msg, outMessage, id, targetId);
}

bool MessageBuilder::addDataToOutMsg(const MarketDataRequest& msg, OutMessage& outMessage, std::string_view id, std::string_view targetId)
{
    return addDataToOutMsgTmp(msg, outMessage, id, targetId);
}

bool MessageBuilder::addDataToOutMsg(const NewOrderSingleMessage& msg, OutMessage& outMessage, std::string_view id, std::string_view targetId)
{
    return addDataToOutMsgTmp(msg, outMessage, id, targetId);
}

void MessageBuilder::printOutMessage(const OutMessage& outMessage, std::ostream& os)
{
    for (std::size_t i = 0; i < outMessage.dataSize_; ++i) {
        if (outMessage.buffer_[i] == SOH) {
            os << "|";
            continue;
        }
        os << outMessage.buffer_[i];
    }
}

bool MessageBuilder::addDataToOutMsg(const LogonMessage& msg, OutMessage& outMessage)
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

    auto st = addTagValue(outMessage, ResetSeqNumFlag::name_, ResetSeqNumFlag::length_, static_cast<char>(toFixType(msg.getResetSeqNumFlag())));

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

bool  MessageBuilder::addDataToOutMsg(const HeartBeatMessage& msg, OutMessage& outMessage)
{
    auto testIdLength = msg.getTestIdLength();

    if (testIdLength > 0) {
        auto st = addTagValue(outMessage, TestReqID::name_, TestReqID::length_, msg.getTestID(), testIdLength);

        if (st != MessageBuilder::UpdateStatus::SUCCESS)
            return false;
    }

    return true;
}

bool MessageBuilder::addDataToOutMsg(const MarketDataRequest& msg, OutMessage& outMessage)
{
    if (!msg.getReqID() || msg.getSymbols().empty() || (msg.getSubscriptionType() == ::SubscriptionRequestType::UNKNOWN) ||
        (msg.getUpdateType() ==  ::UpdateType::UNKNOWN)) {
            return false;
    }

    auto st = addTagValue(outMessage, ReqID::name_, ReqID::length_, msg.getReqID());

    if (st != MessageBuilder::UpdateStatus::SUCCESS)
        return false;

    st = addTagValue(outMessage, SubscriptionRequestType::name_, SubscriptionRequestType::length_, static_cast<char>(toFixType(msg.getSubscriptionType())));

    if (st != MessageBuilder::UpdateStatus::SUCCESS)
        return false;

    st = addTagValue(outMessage, MarketDepth::name_, MarketDepth::length_, msg.getMarketDepth());

    if (st != MessageBuilder::UpdateStatus::SUCCESS)
        return false;

    st = addTagValue(outMessage, UpdateType::name_, UpdateType::length_, static_cast<char>(toFixType(msg.getUpdateType())));

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
        st = addTagValue(outMessage, EntryType::name_, EntryType::length_, static_cast<char>(toFixType(type)));

        if (st != MessageBuilder::UpdateStatus::SUCCESS)
            return false;
    
    }

    return true;
}

bool MessageBuilder::addDataToOutMsg(const NewOrderSingleMessage& msg, OutMessage& outMessage)
{
    if (!msg.getClientOrderID() || msg.getSymbolID() == NoSymbolID || (msg.getSide() == ::OrderSide::UNKNOWN) || msg.getOrderTime() == std::chrono::system_clock::time_point()  ||
        !msg.getOrderVolume() || (msg.getOrderPrice() <= ::Price(0)) || (msg.getOrderType() == ::OrderType::UNKNOWN) || (msg.getTimeInForce() == ::TimeInForce::UNKNOWN)) {
            return false;
    }

    auto st = addTagValue(outMessage, ClOrdID::name_, ClOrdID::length_, msg.getClientOrderID());

    if (st != MessageBuilder::UpdateStatus::SUCCESS)
        return false;

    const auto& allSymbols = getSymbols();
    auto symbolName = allSymbols.getSymbolName(msg.getSymbolID());

    st = addTagValue(outMessage, Symbol::name_, Symbol::length_, symbolName.data(), symbolName.length());

    if (st != MessageBuilder::UpdateStatus::SUCCESS)
        return false;

    st = addTagValue(outMessage, Side::name_, Side::length_, static_cast<char>(toFixType(msg.getSide())));

    if (st != MessageBuilder::UpdateStatus::SUCCESS)
        return false;

    char timeStampBuffer[30];
    auto l = toFixFormat(msg.getOrderTime(), timeStampBuffer, 30, timeAccuracy_);

    if (!l) {
        return false;
    }

    st = addTagValue(outMessage, TransactionTime::name_, TransactionTime::length_, timeStampBuffer, l);

    if (st != MessageBuilder::UpdateStatus::SUCCESS)
        return false;

    st = addTagValue(outMessage, OrderQty::name_, OrderQty::length_, msg.getOrderVolume());

    if (st != MessageBuilder::UpdateStatus::SUCCESS)
        return false;

    st = addTagValue(outMessage, Price::name_, Price::length_, msg.getOrderPrice());

    if (st != MessageBuilder::UpdateStatus::SUCCESS)
        return false;

    st = addTagValue(outMessage, OrderType::name_, OrderType::length_, static_cast<char>(toFixType(msg.getOrderType())));

    if (st != MessageBuilder::UpdateStatus::SUCCESS)
        return false;

    st = addTagValue(outMessage, TimeInForce::name_, TimeInForce::length_, static_cast<char>(toFixType(msg.getTimeInForce())));

    if (st != MessageBuilder::UpdateStatus::SUCCESS)
        return false;

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
    auto timestartPtr = outMessage.buffer_.data() + outMessage.timeStampPos_ + SendingTime::length_ + 1;
    auto timeEndPtr= timestartPtr + timeStampPlaceHolder_.length();

    auto l = toFixFormat(now, timestartPtr, timeStampPlaceHolder_.length(), timeAccuracy_);

    if (!l) {
        return false;
    }

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

MessageBuilder::UpdateStatus MessageBuilder::addTagValue(OutMessage& msg, const char* tag, std::size_t tagLength, ::Price value)
{
    if (msg.buffer_.size() < msg.dataSize_ + tagLength + 1) {
        msg.status_ = OutMessage::MessageStatus::INVALID;
        return UpdateStatus::OVERFLOW;
    }

    memcpy(msg.buffer_.data() + msg.dataSize_, tag, tagLength);
    msg.dataSize_ += tagLength;
    msg.buffer_[msg.dataSize_] = '=';
    ++msg.dataSize_ ;

    auto st = std::to_chars(msg.buffer_.data() + msg.dataSize_, msg.buffer_.data() + msg.buffer_.size(), value.rawValue());

    if (st.ec != std::errc() || st.ptr > msg.buffer_.data() + msg.buffer_.size() - 2 ) {
        msg.status_ = OutMessage::MessageStatus::INVALID;
        return UpdateStatus::OVERFLOW;
    }

    std::memmove(st.ptr - value.getDecimalPlaces() + 1, st.ptr - value.getDecimalPlaces(), value.getDecimalPlaces());
    *(st.ptr - value.getDecimalPlaces()) = '.';
    ++st.ptr;
    *st.ptr = SOH;
    msg.dataSize_ = st.ptr - msg.buffer_.data() + 1;
    return UpdateStatus::SUCCESS;
}

}

