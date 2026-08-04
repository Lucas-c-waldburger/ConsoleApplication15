#pragma once
#include "../ScriptComponent.h"

inline bool ComponentValid(const Script& script)
{
	return script.table.IsValid();
}