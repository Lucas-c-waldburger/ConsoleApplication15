#pragma once
#include "BaseComponent.h"
#include "../scripting/ScriptTable.h"

struct Script : BaseComponent<Script>
{
	ScriptTableView table;

	bool operator==(const Script&) const = default;
};