#pragma once
#include "../SerializationConcepts.h"
#include "../../systems/ScriptSystem.h"
#include "CoreJsonUserTypes.h"

NLOHMANN_JSON_SERIALIZE_ENUM(
	ScriptTable::TableType,
	{
		{ ScriptTable::TableType::Invalid,       "Invalid" },
		{ ScriptTable::TableType::FunctionTable, "FunctionTable" },
		{ ScriptTable::TableType::SystemTable,   "SystemTable" }
	}
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ScriptTableDescriptor, name, filepath, tableType, systemPhase)