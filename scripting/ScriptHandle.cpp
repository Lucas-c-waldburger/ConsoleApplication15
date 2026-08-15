#include "ScriptHandle.h"

std::pair<ScriptTable2::TableId, ScriptTableEntry>
ScriptTable2::CreateTableEntry(const std::string& path, sol::table&& tbl,
							   ParsedLuaFunctionTableSignatures&& fns)
{
	const auto id = tableIdCounter_++;

	return std::pair<TableId, ScriptTableEntry>{
		std::piecewise_construct,
			std::forward_as_tuple(id),
			std::forward_as_tuple(path, ScriptTable2{ std::move(tbl), std::move(fns), id })
	};
}

ScriptTableView2 ScriptTable2::GetView() const
{
	return ScriptTableView2(this);
}