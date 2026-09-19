#include "FixTagValueReader.h"

bool TagValueReader::getTag(int &tag)
{
    tag = 0;
    bool retval = false;

    for (auto i = parsePos_; i != end_; ++i) {
        auto index = i & mask_;
        char val = buffer_[index];

        if (val != '=' ) {
            tag = tag * 10 + (val - '0');
        } else {
            retval = true;
            lastTagPos_ = parsePos_ - start_;
            parsePos_ = i + 1;
            break;
        }
    }
    return retval;
}

bool TagValueReader::getValue(char& value)
{
    bool retval = false;

    for (auto i = parsePos_ ; i != end_; ++i) {
        auto index = i & mask_;
        char val = buffer_[index];

        if (val != SOH ) {
            value = val;
        } else {
            retval = true;
            parsePos_ = i + 1;
            break;
        }
    }
    return retval;
}

int TagValueReader::getValue(char* value, int length)
{
    int retVal = 0;

    for (std::size_t i = parsePos_; i != end_; ++i) {
        auto index = i & mask_;
        char val = buffer_[index];
        ++retVal;
        if (retVal > length)
            return 0;
        if (val != SOH ) {
            value[retVal - 1] = val;
        } else {
            parsePos_ = i + 1;
            value[retVal - 1] = '\0';
            break;
        }
    }
    return retVal;
}

bool TagValueReader::moveReadPosTo(std::size_t offset)
{
    auto newReadPos = start_ + offset;

    if (offset >= end_) {
        return false;
    }

    parsePos_ = newReadPos;
    return true;
}

bool TagValueReader::moveToNextTag()
{

    for (std::size_t i = parsePos_; i != end_; ++i) {
        auto index = i & mask_;
        char val = buffer_[index];

        if (val == SOH ) {
            parsePos_ = i + 1;
            return true;
        } 
    }
    return false;;
}