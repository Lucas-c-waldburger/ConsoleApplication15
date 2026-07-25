#pragma once
#include "BaseComponent.h"

template <typename T>
concept SomeComponent = 
    SomeTypeInList<T, ComponentTypeList> &&
    std::derived_from<T, BaseComponent<T>>;

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