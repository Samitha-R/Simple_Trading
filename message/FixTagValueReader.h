#ifndef FIX_TAG_VALUE_READER_H
#define FIX_TAG_VALUE_READER_H

#include <cstdint>
#include <concepts>
#include "TypeDef.h"
#include "FixTags.h"

class TagValueReader
{
public:
    TagValueReader() = default;
    TagValueReader(const char* buffer, std::size_t start, std::size_t end, std::size_t mask) :
        buffer_(buffer), mask_(mask), start_(start), end_(end), parsePos_(start) , lastTagPos_(start) {}
    bool getTag(int& tag);
    template<std::integral T> bool getValue(T& value);
    template<std::floating_point T> bool getValue(T& value);
    template <std::integral V, unsigned int D>  bool getValude(Decimal<V,D>& value);
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

template<std::integral T> bool TagValueReader::getValue(T& value)
{
    value = 0;
    bool retval = false;

    for (auto i = parsePos_ ; i != end_; ++i) {
        auto index = i & mask_;
        char val = buffer_[index];

        if (val != SOH ) {
            value = value * 10 + (val - '0');
        } else {
            retval = true;
            parsePos_ = i + 1;
            break;
        }
    }
    return retval;
}

template<std::floating_point T> bool TagValueReader::getValue(T& value)
{
    value = 0;
    bool retval = false;
    int decimals = 0;
    bool decimal = false;

    for (auto i = parsePos_ ; i != end_; ++i) {
        auto index = i & mask_;
        char val = buffer_[index];

        if (val != SOH ) {

            if ( val == '.') {
                decimal = true;
                continue;
            }

            value = value * 10 + (val - '0');

            if (decimal)
                ++decimals;

        } else {
            retval = true;
            parsePos_ = i + 1;
            break;
        }
    }

    while (decimals--)
        value = value / 10;

    return retval;
}

template <std::integral V, unsigned int D>  bool TagValueReader::getValude(Decimal<V,D>& value)
{
    value = 0;
    bool retval = false;
    int decimals = 0;
    bool decimal = false;

    for (auto i = parsePos_ ; i != end_; ++i) {
        auto index = i & mask_;
        char val = buffer_[index];

        if (val != SOH ) {

            if ( val == '.') {
                decimal = true;
                continue;
            }

            value = value * 10 + (val - '0');

            if (decimal)
                ++decimals;

        } else {
            parsePos_ = i + 1;
            if (decimals < D) {
                while (decimals < D) {
                    value = value * 10;
                    ++decimals;
                }
                retval = true;
            } else if (decimals == D) {
                retval = true;
            } 
            break;
        }
    }

    return retval;
}

#endif