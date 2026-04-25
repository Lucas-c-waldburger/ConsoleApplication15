#pragma once
#include "data/CoreEventDataTypeList.h"
#include "../user/UserEventTypeList.h"

using EventDataTypeList = concat_type_lists_t<
	CoreEventDataTypeList, UserEventTypeList
>;
