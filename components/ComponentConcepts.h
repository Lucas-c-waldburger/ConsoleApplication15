#pragma once
#include "ComponentTypeList.h"

static constexpr size_t kMaxComponents = 64;

using ComponentSignature = uint64_t;

template <typename T>
concept SomeComponent = SomeTypeInList<std::remove_cvref_t<T>, ComponentTypeList>;

template <SomeComponent T>
struct component_traits {
    static constexpr size_t index = index_of_v<T, ComponentTypeList>;
    static constexpr ComponentSignature bit = 1ull << index;
};

/** @defgroup ComponentId @{ */
/** 
  * index - the component's index into the type list specified in MakeComponentId()
  * bit - the component's bit signature
  */
struct ComponentId
{ 
	size_t index = std::numeric_limits<size_t>::max();
	ComponentSignature bit = 0;

    constexpr operator bool() const noexcept
    {
        return bit != 0 && index < std::numeric_limits<size_t>::max();
    }
    constexpr bool operator!() const noexcept
    {
        return !(*this);
    }
};

template <SomeComponent T, typename TList> requires type_in_list_v<T, TList>
inline constexpr ComponentId MakeComponentId()
{
    return {
        .index = index_of_v<T, TList>,
        .bit = 1ull << index_of_v<T, ComponentTypeList>
    };
}
/** @} */

/** Assigns a "NeedsUpdate" component (UpdateType) that gets added
  * automatically when component T is added through AddComponent() 
  * or Accessed through non-const GetComponent() 
  */
template <SomeComponent T>
struct TriggersUpdate 
{ 
    using UpdateType = T; 
    constexpr bool operator==(const TriggersUpdate&) const = default;
};

template <typename T>
concept SomeUpdateTriggeringComponent = requires() {
    SomeComponent<T>;
    SomeComponent<typename T::UpdateType>;
};
/**/