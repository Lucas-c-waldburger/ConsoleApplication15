#pragma once
#include "ComponentTypeList.h"

static constexpr size_t kMaxComponents = 64;

using ComponentSignature = uint64_t;

static constexpr uint64_t kReservedComponentBit = 0;


template <typename Derived> requires type_in_list_v<Derived, ComponentTypeList>
struct BaseComponent
{
    static constexpr uint64_t componentBit = 
        1ull << index_of_v<Derived, ComponentTypeList>;
};




