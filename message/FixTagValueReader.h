#ifndef FIX_TAG_VALUE_READER_H
#define FIX_TAG_VALUE_READER_H

#include <cstdint>

class TagValueReader
{
public:
    TagValueReader() = default;
    TagValueReader(const char* buffer, std::size_t start, std::size_t end, std::size_t mask) :
        buffer_(buffer), mask_(mask), start_(start), end_(end), parsePos_(start) , lastTagPos_(start) {}
    bool getTag(int& tag);
    bool getValue(int& value);
    bool getValue(double& value);
    bool getValue(char& value);
    int getValue(char* value, int length);
    bool moveReadPosTo(std::size_t offset);
    bool moveToNextTag();
    bool empty() { return start_ == end_; }
    std::size_t getLastTagPos() { return lastTagPos_ & mask_; }
    std::size_t getParseBytes() { return parsePos_ - start_; }
    std::size_t getRemainBytes() { return end_ - parsePos_; }
    char operator[](std::size_t i) { return buffer_[(i + start_) & mask_];}

private:
    const char* buffer_ = nullptr;
    std::size_t mask_ = 0;
    std::size_t start_ = 0;
    std::size_t end_ = 0;
    std::size_t parsePos_ = 0;
    std::size_t lastTagPos_ = 0;
};

#endif