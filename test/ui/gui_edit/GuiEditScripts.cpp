#include "GuiEditScripts.h"

#if IMGUI_ENABLED
#include <filesystem>

namespace ui {

namespace fs = std::filesystem;

namespace {

std::string GetFileName(const std::string& filepath)
{
	auto path = fs::path(filepath);
	if (fs::exists(path))
	{
		return path.filename().string();
	}
	return {};
}

} // unnamed

PropertyEditState GuiEditProperty(ScriptFilepathsContext& ctx)
{
	auto curName = GetFileName(ctx.selectedFilepath);

	if (ImGui::BeginCombo("##SFC", curName.c_str()))
	{
		for (size_t i = 0; i < ctx.package.size(); ++i)
		{
			const auto& filepath = ctx.package[i].filepath;

			const bool selected = (filepath == ctx.selectedFilepath);

			auto filename = GetFileName(filepath);

			if (ImGui::Selectable(filename.c_str(), &selected))
			{
				ctx.selectedFilepath = filepath;
				ctx.packageIndex = i;
			}
		}

		ImGui::EndCombo();
	}

	return EvaluatePropertyState();
}

PropertyEditState GuiEditProperty(ScriptTableFunctionNamesContext& ctx)
{
	auto curName = GetFileName(ctx.selectedTableFunction);

	if (ImGui::BeginCombo("##STFNC", curName.c_str()))
	{
		for (const auto& fnName : ctx.tableFunctionNames)
		{
			const bool selected = (fnName == ctx.selectedTableFunction);

			if (ImGui::Selectable(fnName.c_str(), &selected))
			{
				ctx.selectedTableFunction = fnName;
			}
		}

		ImGui::EndCombo();
	}

	return EvaluatePropertyState();
}

} // ui

#endif