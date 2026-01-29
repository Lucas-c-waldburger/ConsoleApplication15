#pragma once

namespace events {

struct UserEvent0;
struct UserEvent1;
struct UserEvent2;
struct UserEvent3;
struct UserEvent4;
struct UserEvent5;

}

using UserEventTypeList = TypeList<
	events::UserEvent0,
	events::UserEvent1,
	events::UserEvent2,
	events::UserEvent3,
	events::UserEvent4,
	events::UserEvent5
>;