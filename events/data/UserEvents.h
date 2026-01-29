#pragma once
#include "../IEventData.h"
#include "../../core/InlineStorage.h"

static constexpr size_t kUserEventStorageSize = 32;

template <typename Derived>
struct UserEventBase : IEventData<Derived>
{
	InlineStorage<kUserEventStorageSize> data;
};

namespace events {

struct UserEvent0 : UserEventBase<UserEvent0> {};
struct UserEvent1 : UserEventBase<UserEvent1> {};
struct UserEvent2 : UserEventBase<UserEvent2> {};
struct UserEvent3 : UserEventBase<UserEvent3> {};
struct UserEvent4 : UserEventBase<UserEvent4> {};
struct UserEvent5 : UserEventBase<UserEvent5> {};

}

#define USER_EVENT_X_LIST(X) \
	X(events::UserEvent0) \
	X(events::UserEvent1) \
	X(events::UserEvent2) \
	X(events::UserEvent3) \
	X(events::UserEvent4) \
	X(events::UserEvent5) 