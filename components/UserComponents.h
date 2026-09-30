#pragma once
#include "ComponentConcepts.h"
#include "../core/InlineStorage.h"
#include "../user/UserComponentTypeList.h"

static constexpr size_t kUserComponentStorageSize = 64;

template <typename T>
concept SomeUserComponent = SomeTypeInList<std::remove_cvref_t<T>, UserComponentTypeList>;

template <SomeUserComponent T>
struct user_component_traits
{
	static constexpr size_t index = index_of_v<T, UserComponentTypeList>;
};

struct UserComponent0 { InlineStorage<kUserComponentStorageSize> data; };
struct UserComponent1 { InlineStorage<kUserComponentStorageSize> data; };
struct UserComponent2 { InlineStorage<kUserComponentStorageSize> data; };
struct UserComponent3 { InlineStorage<kUserComponentStorageSize> data; };
struct UserComponent4 { InlineStorage<kUserComponentStorageSize> data; };
struct UserComponent5 { InlineStorage<kUserComponentStorageSize> data; };
struct UserComponent6 { InlineStorage<kUserComponentStorageSize> data; };
struct UserComponent7 { InlineStorage<kUserComponentStorageSize> data; };
struct UserComponent8 { InlineStorage<kUserComponentStorageSize> data; };
struct UserComponent9 { InlineStorage<kUserComponentStorageSize> data; };