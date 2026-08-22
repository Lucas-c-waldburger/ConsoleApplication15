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
	std::string curName;
	auto currTableIter = ctx.scriptTableMap.find(ctx.selectedTableId);
	if (currTableIter != ctx.scriptTableMap.end())
	{
		curName = GetFileName(currTableIter->second.filepath);
	}
	else
	{
		ctx.selectedTableId = std::numeric_limits<size_t>::max();
	}

	if (ImGui::BeginCombo("##SFC", curName.c_str()))
	{
		for (const auto& [id, data] : ctx.scriptTableMap)
		{
			const bool selected = (id == ctx.selectedTableId);

			auto filename = GetFileName(data.filepath);

			if (ImGui::Selectable(filename.c_str(), &selected))
			{
				ctx.selectedTableId = id;
			}
		}

		ImGui::EndCombo();
	}

	return EvaluatePropertyState();
}

PropertyEditState GuiEditProperty(ScriptTableFunctionNamesContext& ctx)
{
	auto currTableIter = ctx.scriptTableMap.find(ctx.selectedTableId);
	if (currTableIter == ctx.scriptTableMap.end())
	{
		ctx.selectedTableFunction.clear();
	}

	if (ImGui::BeginCombo("##STFNC", ctx.selectedTableFunction.c_str()))
	{
		if (currTableIter != ctx.scriptTableMap.end())
		{
			const auto& fnSigs = currTableIter->second.table.GetFunctionSignatures();

			for (const auto& [fnName, _] : fnSigs)
			{
				const bool selected = (fnName == ctx.selectedTableFunction);

				if (ImGui::Selectable(fnName.c_str(), &selected))
				{
					ctx.selectedTableFunction = fnName;
				}
			}
		}

		ImGui::EndCombo();
	}

	return EvaluatePropertyState();
}

} // ui

#endif