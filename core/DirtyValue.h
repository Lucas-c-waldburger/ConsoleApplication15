#pragma once
#include <algorithm>
#include <iostream>
#include <cassert>
#include <concepts>
#include <ranges>
#include <vector>

namespace detail {
template <typename T, typename U> concept SupportsAdd = requires(const T & t, const U & u) { t + u; };
template <typename T, typename U> concept SupportsSub = requires(const T & t, const U & u) { t - u; };
template <typename T, typename U> concept SupportsMult = requires(const T & t, const U & u) { t* u; };
template <typename T, typename U> concept SupportsDiv = requires(const T & t, const U & u) { t / u; };
template <typename T, typename U> concept SupportsPlusEq = requires(T & t, const U & u) { t += u; };
template <typename T, typename U> concept SupportsSubEq = requires(T & t, const U & u) { t -= u; };
template <typename T, typename U> concept SupportsMultEq = requires(T & t, const U & u) { t *= u; };
template <typename T, typename U> concept SupportsDivEq = requires(T & t, const U & u) { t /= u; };
template <typename T, typename U> concept SupportsModEq = requires(T & t, const U & u) { t %= u; };
template <typename T> concept SupportsUnaryMin = requires(const T & t) { -t; };

template <typename T> concept SupportsPreInc = requires(T & t) { ++t; };
template <typename T> concept SupportsPostInc = requires(T & t) { t++; };
template <typename T> concept SupportsPreDec = requires(T & t) { --t; };
template <typename T> concept SupportsPostDec = requires(T & t) { t--; };

template <typename T, typename U> concept SupportsMutBracket = requires(T & t, const U & u) { t[u]; };
template <typename T, typename U> concept SupportsConstBracket = requires(const T & t, const U & u) { t[u]; };

template <typename T, typename U> concept SupportsModulo = requires(const T & t, const U & u) { t% u; };

template <typename T> concept SupportsMutBegin = requires(T & t) { t.begin(); };
template <typename T> concept SupportsConstBegin = requires(const T & t) { t.begin(); };
template <typename T> concept SupportsMutEnd = requires(T & t) { t.end(); };
template <typename T> concept SupportsConstEnd = requires(const T & t) { t.end(); };

template <typename T> concept SupportsSize = requires(const T & t) { t.size(); };
template <typename T> concept SupportsClear = requires(T & t) { t.clear(); };
template <typename T> concept SupportsEmpty = requires(const T & t) { t.empty(); };

template <typename T> concept RangeLike = SupportsMutBegin<T> && SupportsConstBegin<T> && SupportsMutEnd<T> && SupportsConstEnd<T>;

template <typename T> concept SupportsOpBool = requires(const T & t) { { t } -> std::convertible_to<bool>; };
template <typename T> concept SupportsOpBoolNot = requires(const T & t) { { !t } -> std::convertible_to<bool>; };

template <typename T, typename U> concept SupportsBitwiseOr = requires(const T & t, const U & u) { t | u; };
template <typename T, typename U> concept SupportsBitwiseAnd = requires(const T & t, const U & u) { t& u; };
template <typename T, typename U> concept SupportsBitwiseHat = requires(const T & t, const U & u) { t^ u; };
template <typename T, typename U> concept SupportsBitwiseOrEq = requires(T & t, const U & u) { t |= u; };
template <typename T, typename U> concept SupportsBitwiseAndEq = requires(T & t, const U & u) { t &= u; };
template <typename T, typename U> concept SupportsBitwiseHatEq = requires(T & t, const U & u) { t ^= u; };

template <typename T, typename U> concept SupportsLShift = requires(const T & t, const U & u) { t << u; };
template <typename T, typename U> concept SupportsRShift = requires(const T & t, const U & u) { t >> u; };
template <typename T, typename U> concept SupportsLShiftEq = requires(T & t, const U & u) { t <<= u; };
template <typename T, typename U> concept SupportsRShiftEq = requires(T & t, const U & u) { t >>= u; };
} // detail

template <typename T>
class DirtyValue
{
public:
    constexpr DirtyValue() requires std::is_default_constructible_v<T> = default;
    template <typename...Args> requires std::constructible_from<T, Args...>
    constexpr DirtyValue(Args&&...args) noexcept(std::is_nothrow_constructible_v<T, Args...>) :
        value_(std::forward<Args>(args)...) {
    }
    ~DirtyValue() = default;

    template <typename U> requires std::convertible_to<U, T>
    constexpr DirtyValue& operator=(U&& val) noexcept
    {
        if (value_ != val)
        {
            value_ = std::forward<U>(val);
            dirty_ = true;
        }
        return *this;
    }

    constexpr DirtyValue(const DirtyValue& other) requires std::copy_constructible<T> :
        value_(other.value_), dirty_(true) {}
    constexpr DirtyValue(DirtyValue&& other) noexcept requires std::move_constructible<T> :
        value_(std::move(other.value_)), dirty_(true) {}
    constexpr DirtyValue& operator=(const DirtyValue& other) requires std::is_copy_assignable_v<T>
    {
        if (this != other && value_ != other.value_)
        {
            value_ = other.value_;
            dirty_ = true;
        }
        return *this;
    }
    constexpr DirtyValue& operator=(DirtyValue&& other) noexcept requires std::is_move_assignable_v<T>
    {
        if (this != other && value_ != other.value_)
        {
            value_ = std::move(other.value_);
            dirty_ = true;
        }
        return *this;
    }

    constexpr T& write() { dirty_ = true; return value_; }
    constexpr const T& read() const { return value_; }

    constexpr operator const T& () const noexcept { return value_; }

    constexpr bool operator==(const T& other) const requires std::equality_comparable<T> { return value_ == other; }
    constexpr auto operator<=>(const T& other) const requires std::three_way_comparable<T> { return value_ <=> other; }

    constexpr bool operator==(const DirtyValue& other) const requires std::equality_comparable<T> { return value_ == other.value_; }
    constexpr auto operator<=>(const DirtyValue& other) const requires std::three_way_comparable<T> { return value_ <=> other.value_; }

    template <typename U> constexpr decltype(auto) operator+(const U& u) const requires detail::SupportsAdd<T, U> { return value_ + u; }
    template <typename U> constexpr decltype(auto) operator-(const U& u) const requires detail::SupportsSub<T, U> { return value_ - u; }
    template <typename U> constexpr decltype(auto) operator*(const U& u) const requires detail::SupportsMult<T, U> { return value_ * u; }
    template <typename U> constexpr decltype(auto) operator/(const U& u) const requires detail::SupportsDiv<T, U> { return value_ / u; }

    template <typename U> constexpr decltype(auto) operator+=(const U& u) requires detail::SupportsPlusEq <T, U> { return write() += u; }
    template <typename U> constexpr decltype(auto) operator-=(const U& u) requires detail::SupportsSubEq <T, U> { return write() -= u; }
    template <typename U> constexpr decltype(auto) operator*=(const U& u) requires detail::SupportsMultEq <T, U> { return write() *= u; }
    template <typename U> constexpr decltype(auto) operator/=(const U& u) requires detail::SupportsDivEq <T, U> { return write() /= u; }

    constexpr decltype(auto) operator-() const requires detail::SupportsUnaryMin<T> { return -value_; }

    constexpr decltype(auto) operator++() requires detail::SupportsPreInc<T> { return ++write(); }
    constexpr decltype(auto) operator++(int) requires detail::SupportsPostInc<T> { return write()++; }
    constexpr decltype(auto) operator--() requires detail::SupportsPreDec<T> { return --write(); }
    constexpr decltype(auto) operator--(int) requires detail::SupportsPostDec<T> { return write()--; }

    constexpr operator bool() const requires detail::SupportsOpBool<T> { return value_; }
    constexpr bool operator!() const requires detail::SupportsOpBoolNot<T> { return !value_; }

    template <typename U> constexpr decltype(auto) operator[](const U& u) requires detail::SupportsMutBracket<T, U> { return write()[u]; }
    template <typename U> constexpr decltype(auto) operator[](const U& u) const requires detail::SupportsConstBracket<T, U> { return value_[u]; }

    template <typename U> constexpr decltype(auto) operator%(const U& u) const requires detail::SupportsModulo<T, U> { return value_ % u; }

    template <typename U> constexpr decltype(auto) operator&(const U& u) const requires detail::SupportsBitwiseAnd<T, U> { return value_ & u; }
    template <typename U> constexpr decltype(auto) operator|(const U& u) const requires detail::SupportsBitwiseOr<T, U> { return value_ | u; }
    template <typename U> constexpr decltype(auto) operator^(const U& u) const requires detail::SupportsBitwiseHat<T, U> { return value_ ^ u; }
    template <typename U> constexpr decltype(auto) operator&=(const U& u) requires detail::SupportsBitwiseAndEq<T, U> { return write() &= u; }
    template <typename U> constexpr decltype(auto) operator|=(const U& u) requires detail::SupportsBitwiseOrEq<T, U> { return write() |= u; }
    template <typename U> constexpr decltype(auto) operator^=(const U& u) requires detail::SupportsBitwiseHatEq<T, U> { return write() ^= u; }

    template <typename U> constexpr decltype(auto) operator<<(const U& u) const requires detail::SupportsLShift<T, U> { return value_ << u; }
    template <typename U> constexpr decltype(auto) operator>>(const U& u) const requires detail::SupportsRShift<T, U> { return value_ >> u; }
    template <typename U> constexpr decltype(auto) operator<<=(const U& u) requires detail::SupportsLShiftEq<T, U> { return write() <<= u; }
    template <typename U> constexpr decltype(auto) operator>>=(const U& u) requires detail::SupportsRShiftEq<T, U> { return write() >>= u; }

    template <typename U> friend constexpr bool EvalAndCleanDirty(DirtyValue<U>& dv);

private:
    T value_;
    bool dirty_ = true;
};

template <typename T>
inline constexpr bool EvalAndCleanDirty(DirtyValue<T>& dv)
{
    if (dv.dirty_)
    {
        dv.dirty_ = false;
        return true;
    }
    return false;
}