#ifndef DECIMAL_H
#define DECIMAL_H

#include <concepts>
#include <cstdint>
#include <ostream>
#include <type_traits>
#include <utility>

constexpr std::size_t inpow10(std::size_t exponent)
{
    std::size_t result = 1;

    for (unsigned int i = 0; i < exponent; ++i) {
        result *= 10;
    }

    return result;
}

template <std::integral V, unsigned int D> class Decimal
{
    template <std::integral V1, unsigned int D1, std::integral V2, unsigned int D2> friend auto operator+(const Decimal<V1,D1> &l, const Decimal<V2,D2>& r);
    template <std::integral V1, unsigned int D1, std::integral V2, unsigned int D2> friend auto operator-(const Decimal<V1,D1> &l, const Decimal<V2,D2>& r);
    template <std::integral V1, unsigned int D1, std::integral V2, unsigned int D2> friend auto operator/(const Decimal<V1,D1> &l, const Decimal<V2,D2>& r);
    template <std::integral V1, unsigned int D1, std::integral V2, unsigned int D2> friend auto operator*(const Decimal<V1,D1> &l, const Decimal<V2,D2>& r);
    template <std::integral V1, unsigned int D1, std::integral N> friend Decimal<V1,D1> operator*(const Decimal<V1,D1>& d, N n);
    template <std::integral V1, unsigned int D1, std::integral N> friend Decimal<V1,D1> operator*(N n, const Decimal<V1,D1>& d);
    template <std::integral V1, unsigned int D1, std::integral N> friend Decimal<V1,D1> operator/(const Decimal<V1,D1>& d, N n);
    template <std::integral V1, unsigned int D1, std::integral N> friend Decimal<V1,D1> operator/(N n, const Decimal<V1,D1>& d);
    template <std::integral V1, unsigned int D1, std::integral N> friend Decimal<V1,D1> operator+(const Decimal<V1,D1>& d, N n);
    template <std::integral V1, unsigned int D1, std::integral N> friend Decimal<V1,D1> operator+(N n, const Decimal<V1,D1>& d);
    template <std::integral V1, unsigned int D1, std::integral N> friend Decimal<V1,D1> operator-(const Decimal<V1,D1>& d, N n);
    template <std::integral V1, unsigned int D1, std::integral N> friend Decimal<V1,D1> operator-(N n, const Decimal<V1,D1>& d);
    template <std::integral V1, unsigned int D1> friend std::ostream& operator<<(std::ostream& os, const Decimal<V1,D1>& d);
public:
    constexpr Decimal(std::size_t value = 0) : value_(value) {}
    Decimal(const Decimal<V,D>& d) = default;
    template <std::integral V2, unsigned int D2>  Decimal(const Decimal<V2,D2>& d);
    template <std::integral N> Decimal<V,D>& operator*=(N n);
    template <std::integral N> Decimal<V,D>& operator/=(N n);
    template <std::integral N> Decimal<V,D>& operator+=(N n);
    template <std::integral N> Decimal<V,D>& operator-=(N n);
    Decimal<V,D>& operator+=(const Decimal<V,D>& d);
    Decimal<V,D>& operator-=(const Decimal<V,D>& d);
    Decimal<V,D>& operator*=(const Decimal<V,D>& d);
    Decimal<V,D>& operator/=(const Decimal<V,D>& d);
    template <std::integral V2, unsigned int D2> auto operator<=>(const Decimal<V2,D2>& d) const;
    template <std::integral V2, unsigned int D2> auto operator==(const Decimal<V2,D2>& d) const;
    template <std::integral V2, unsigned int D2> Decimal<V,D>& operator=(const Decimal<V2,D2>& d);
    V rawValue() const { return value_; }
private:
    V value_;
};

template <std::integral V, unsigned int D> template <std::integral V2, unsigned int D2>  Decimal<V,D>::Decimal(const Decimal<V2,D2>& d)
{
    if constexpr (D2 > D) {
        value_ = d.rawValue() / inpow10(D2 - D);
    } else if constexpr (D2 < D) {
        value_ = d.rawValue() * inpow10(D - D2);
    } else {
        value_ = d.rawValue();
    }
}

template <std::integral V, unsigned int D> template <std::integral V2, unsigned int D2> Decimal<V,D>& Decimal<V,D>::operator=(const Decimal<V2,D2>& d)
{
    if constexpr (D2 > D) {
        value_ = d.rawValue() / inpow10(D2 - D);
    } else if constexpr (D2 < D) {
        value_ = d.rawValue() * inpow10(D - D2);
    } else {
        value_ = d.rawValue();
    }
    return *this;
}

template <std::integral V, unsigned int D> template <std::integral N> Decimal<V,D>& Decimal<V,D>::operator*=(N n) {
    value_ *= n;
    return *this;
}

template <std::integral V, unsigned int D> template <std::integral N> Decimal<V,D>& Decimal<V,D>::operator/=(N n) 
{
    value_ /= n;
    return *this;
}

template <std::integral V, unsigned int D> template <std::integral N> Decimal<V,D>& Decimal<V,D>::operator+=(N n) 
{
    value_ += n * inpow10(D);
    return *this;
}

template <std::integral V, unsigned int D> template <std::integral N> Decimal<V,D>& Decimal<V,D>::operator-=(N n) 
{
    value_ -= n * inpow10(D);
    return *this;
}

template <std::integral V1, unsigned int D1, std::integral N> Decimal<V1,D1> operator*(const Decimal<V1,D1>& d, N n)
{
    Decimal ans(d);
    return ans *= n;
    return ans;
}

template <std::integral V1, unsigned int D1, std::integral N> Decimal<V1,D1> operator*(N n, const Decimal<V1,D1>& d)
{
    Decimal ans(d);
    return ans *= n;
    return ans;
}

template <std::integral V1, unsigned int D1, std::integral N> Decimal<V1,D1> operator/(const Decimal<V1,D1>& d, N n)
{
    Decimal ans(d);
    return ans /= n;
}

template <std::integral V1, unsigned int D1, std::integral N> Decimal<V1,D1> operator/(N n, const Decimal<V1,D1>& d)
{
    Decimal ans(n * inpow10(D1) / d.rawValue());
    return ans;
}

template <std::integral V1, unsigned int D1, std::integral N> Decimal<V1,D1> operator+(const Decimal<V1,D1>& d, N n)
{
    Decimal ans(d);
    return ans += n;
}

template <std::integral V1, unsigned int D1, std::integral N> Decimal<V1,D1> operator+(N n, const Decimal<V1,D1>& d)
{
    Decimal ans(d);
    return ans += n;
}

template <std::integral V1, unsigned int D1, std::integral N> Decimal<V1,D1> operator-(const Decimal<V1,D1>& d, N n)
{
    Decimal ans(d);
    return ans -= n;
}

template <std::integral V1, unsigned int D1, std::integral N> Decimal<V1,D1> operator-(N n, const Decimal<V1,D1>& d)
{
    Decimal ans(n * inpow10(D1));
    return ans -= d;
}

template <std::integral V, unsigned int D> Decimal<V,D>& Decimal<V,D>::operator+=(const Decimal<V,D>& d)
{
    value_ += d.value_;
    return *this;
}

template <std::integral V, unsigned int D> Decimal<V,D>& Decimal<V,D>::operator-=(const Decimal<V,D>& d)
{
    value_ -= d.value_;
    return *this;
}

template <std::integral V, unsigned int D> Decimal<V,D>& Decimal<V,D>::operator*=(const Decimal<V,D>& d)
{
    value_ *= d.value_ / inpow10(D);
    return *this;
}

template <std::integral V, unsigned int D> Decimal<V,D>& Decimal<V,D>::operator/=(const Decimal<V,D>& d)
{
    value_ = (value_ * inpow10(D)) / d.value_;
    return *this;
}

template <std::integral V, unsigned int D> template<std::integral V2, unsigned int D2> auto Decimal<V,D>::operator<=>(const Decimal<V2,D2>& d) const
{
    if constexpr (std::is_same_v<V,V2> && (D == D2)) {
        return value_ <=> d.rawValue();
    } else {
        using ResultType = Decimal<typename std::common_type_t<V, V2>, (D > D2) ? D : D2>;
        ResultType lCp(*this);
        ResultType rCp(d);
        return lCp.value_ <=> rCp.value_; 
    }
}

template <std::integral V, unsigned int D> template<std::integral V2, unsigned int D2> auto Decimal<V,D>::operator==(const Decimal<V2,D2>& d) const
{
    if constexpr (std::is_same_v<V,V2> && (D == D2)) {
        return value_ == d.rawValue();
    } else {
        using ResultType = Decimal<typename std::common_type_t<V, V2>, (D > D2) ? D : D2>;
        ResultType lCp(*this);
        ResultType rCp(d);
        return lCp.value_ == rCp.value_; 
    }
}

template <std::integral V1, unsigned int D1, std::integral V2, unsigned int D2> auto operator+(const Decimal<V1,D1> &l, const Decimal<V2,D2>& r)
{
    if constexpr (std::is_same_v<V1,V2> && (D1 == D2)) {
        Decimal<V1,D1> lCp(l);
        lCp += r;
        return lCp;
    } else {
        using ResultType = Decimal<typename std::common_type_t<V1, V2>, (D1 > D2) ? D1 : D2>;
        ResultType lCp(l);
        ResultType rCp(r);
        lCp += rCp;
        return lCp;
    }
}

template <std::integral V1, unsigned int D1, std::integral V2, unsigned int D2> auto operator-(const Decimal<V1,D1> &l, const Decimal<V2,D2>& r)
{
    if constexpr (std::is_same_v<V1,V2> && (D1 == D2)) {
        Decimal<V1,D1> lCp(l);
        lCp -= r;
        return lCp;
    } else {
        using ResultType = Decimal<typename std::common_type_t<V1, V2>, (D1 > D2) ? D1 : D2>;
        ResultType lCp(l);
        ResultType rCp(r);
        lCp -= rCp;
        return lCp;

    }
}

template <std::integral V1, unsigned int D1, std::integral V2, unsigned int D2> auto operator*(const Decimal<V1,D1> &l, const Decimal<V2,D2>& r)
{
    if constexpr (std::is_same_v<V1,V2> && (D1 == D2)) {
        Decimal<V1,D1> lCp(l);
        lCp *= r;
        return lCp;
    } else {
        using ResultType = Decimal<typename std::common_type_t<V1, V2>, (D1 > D2) ? D1 : D2>;
        ResultType lCp(l);
        ResultType rCp(r);
        lCp *= rCp;
        return lCp;
    }
}

template <std::integral V1, unsigned int D1, std::integral V2, unsigned int D2> auto operator/(const Decimal<V1,D1> &l, const Decimal<V2,D2>& r)
{
    if constexpr (std::is_same_v<V1,V2> && (D1 == D2)) {
        Decimal<V1,D1> lCp(l);
        lCp /= r;
        return lCp;
    } else {
        using ResultType = Decimal<typename std::common_type_t<V1, V2>, (D1 > D2) ? D1 : D2>;
        ResultType lCp(l);
        ResultType rCp(r);
        lCp /= rCp;
        return lCp;
    }
}

template <std::integral V1, unsigned int D1> std::ostream& operator<<(std::ostream& os, const Decimal<V1,D1>& d)
{
    return os << d.value_ << ":" << D1 << ":" << static_cast<double>(d.value_) / inpow10(D1);
}

#endif