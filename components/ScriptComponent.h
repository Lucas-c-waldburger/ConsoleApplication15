#pragma once
#include "ComponentConcepts.h"
#include "../scripting/ScriptTable.h"

struct Script
{
	ScriptTableView table;

	bool operator==(const Script&) const = default;
};