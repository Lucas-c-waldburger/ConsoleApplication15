#pragma once
#include "CoreComponentTypeList.h"
#include "../user/UserComponentTypeList.h"

using ComponentTypeList = 
	concat_type_lists_t<CoreComponentTypeList, UserComponentTypeList>;

static_assert(ComponentTypeList::size <= 64, "Component bits cannot exceed 64");