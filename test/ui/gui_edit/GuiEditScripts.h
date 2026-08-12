#include "../../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "../PropertyEditState.h"
#include "../../../systems/ScriptSystem.h"

namespace ui {

struct ScriptFilepathsContext
{
	const ScriptDataPackage& package;
	std::string& selectedFilepath;
	size_t packageIndex = std::numeric_limits<size_t>::max();
};

struct ScriptTableFunctionNamesContext
{
	const std::vector<std::string>& tableFunctionNames;
	std::string& selectedTableFunction;
};

PropertyEditState GuiEditProperty(ScriptFilepathsContext& ctx);
PropertyEditState GuiEditProperty(ScriptTableFunctionNamesContext& ctx);

} // ui

#endif