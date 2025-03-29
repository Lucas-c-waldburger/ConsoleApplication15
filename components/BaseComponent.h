#pragma once
#include "../core/TypeUtils.h"

static constexpr size_t kMaxComponents = 64;

using ComponentSignature = uint64_t;

static constexpr uint64_t kReservedComponentBit = 0;

template <typename Derived, size_t Idx>
struct BaseComponent
{
    static constexpr uint64_t componentBit = 1ull << Idx;
};




