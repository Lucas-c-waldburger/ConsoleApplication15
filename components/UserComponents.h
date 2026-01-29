#pragma once
#include "BaseComponent.h"
#include "../core/InlineStorage.h"
#include "../user/UserComponentTypeList.h"

static constexpr size_t kUserComponentStorageSize = 64;

template <typename Derived>
struct UserComponentBase : BaseComponent<Derived>
{ 
	static constexpr size_t userComponentId =
		index_of_v<Derived, UserComponentTypeList>;

	InlineStorage<kUserComponentStorageSize> data; 
};

template <typename T>
concept SomeUserComponent = std::derived_from<T, UserComponentBase<T>>;

struct UserComponent0 : UserComponentBase<UserComponent0> {};
struct UserComponent1 : UserComponentBase<UserComponent1> {};
struct UserComponent2 : UserComponentBase<UserComponent2> {};
struct UserComponent3 : UserComponentBase<UserComponent3> {};
struct UserComponent4 : UserComponentBase<UserComponent4> {};
struct UserComponent5 : UserComponentBase<UserComponent5> {};