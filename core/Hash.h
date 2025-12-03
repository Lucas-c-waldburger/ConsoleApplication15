#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include "../deps/rapidhash/rapidhash.h"

using HashType = uint64_t;

inline HashType RapidHash(const char* s) noexcept
{
    return rapidhash(s, std::strlen(s));
}

inline HashType RapidHash(std::string_view sv) noexcept
{
    return rapidhash(sv.data(), sv.size());
}

inline HashType RapidHash(const std::string& s) noexcept
{
    return rapidhash(s.data(), s.size());
}

template <typename T>
concept RapidHashable = requires(T t) {
    { RapidHash(t) } -> std::same_as<HashType>;
};

// heterogenous lookup
struct RapidHashTransparentHash {
    using is_transparent = void;

    HashType operator()(HashType k) const noexcept { return k; }

    template <RapidHashable T>
    HashType operator()(const T& t) const noexcept {
        return RapidHash(t);
    }
};

struct RapidHashTransparentEq {
    using is_transparent = void;

    bool operator()(HashType a, HashType b) const noexcept {
        return a == b;
    }

    template <RapidHashable T> 
    bool operator()(HashType a, const T& b) const noexcept {
        return a == RapidHash(b);
    }

    template <RapidHashable T>
    bool operator()(const T& a, HashType b) const noexcept {
        return RapidHash(a) == b;
    }
};

template <typename ValueType>
using RapidHashUnorderedMap = std::unordered_map<HashType, ValueType, 
                                                 RapidHashTransparentHash,
                                                 RapidHashTransparentEq>;