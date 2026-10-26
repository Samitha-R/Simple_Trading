#ifndef OUT_MESSAGE_H
#define OUT_MESSAGE_H

#include <ostream>
#include <vector>

struct OutMessage
{
   enum class MessageStatus { INITIAL, DATA_FILLED, FINALIZED, INVALID };

   OutMessage(std::size_t size = 2048) : buffer_(size) {}
   
   inline const char* getDataToBeSent(size_t &length) const;
   MessageStatus getSatus() { return status_;}
   void addSentCount(size_t size) { sentCount_ += sentCount_;}
   inline void reset();

   std::vector<char> buffer_;
   std::size_t dataSize_ = 0;
   std::size_t sentCount_ = 0;
   MessageStatus status_ = MessageStatus::INITIAL;
   std::size_t msgLenPos_;
   std::size_t seqNumPos_;
   std::size_t timeStampPos_;
};

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

inline std::ostream& operator<<(std::ostream& os, const OutMessage& message)
{
    for (std::size_t i = 0; i < message.dataSize_; ++i) {
        std::cout << message.buffer_[i];
    }
    std::cout << std::endl;
    return os;
}

#endif