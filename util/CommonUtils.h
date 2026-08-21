#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H

#include <cstdint>
#include <map>
#include <vector>
#include <string>
#include <TypeDef.h>
#include <concepts>

template<auto N> concept PowerOfTwo = (N > 0) && ((N & (N - 1)) == 0);

constexpr  bool isPowerOfTwo(std::integral auto n) {
    return (n > 0) && ((n & (n - 1)) == 0);
}

inline std::size_t getNextPowerOfTwo(std::size_t number)
{
    if (number == 0)
        return 1;

    --number;
    number |= number >> 1;
    number |= number >> 2;
    number |= number >> 4;
    number |= number >> 8;
    number |= number >> 16;
    number |= number >> 32;
    return number + 1;
}

class TradeSymbols
{
public:
    TradeSymbols() = default;
    SymbolID addSymbol(std::string_view symbolName);
    std::size_t getNumSymbols() const { return symbolIdvsName_.size(); }
    std::string_view getSymbolName (SymbolID id) const { return symbolIdvsName_[id]; }
    inline SymbolID getSymbolID (const std::string &name) const;
private:
    std::map<std::string,SymbolID> symbolNameVsIDs_;
    std::vector<std::string> symbolIdvsName_;
};

SymbolID TradeSymbols::getSymbolID(const std::string &name) const
{
    auto iter = symbolNameVsIDs_.find(name);

    if (iter != symbolNameVsIDs_.end())
        return iter->second;

    return -1;
}

#endif