#include "../../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "../PropertyEditState.h"
#include "../../../systems/ScriptSystem.h"

namespace ui {

struct ScriptFilepathsContext
{
	const ScriptSystem::ScriptTableMap& scriptTableMap;
	ScriptTable::TableId& selectedTableId;
};

struct ScriptTableFunctionNamesContext
{
	const ScriptSystem::ScriptTableMap& scriptTableMap;
	ScriptTable::TableId selectedTableId;
	std::string& selectedTableFunction;
};

PropertyEditState GuiEditProperty(ScriptFilepathsContext& ctx);
PropertyEditState GuiEditProperty(ScriptTableFunctionNamesContext& ctx);

} // ui

#endif