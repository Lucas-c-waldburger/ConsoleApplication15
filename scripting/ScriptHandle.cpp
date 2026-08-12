#include "ScriptHandle.h"

std::pair<ScriptTable2::TableId, ScriptTable2>
ScriptTable2::CreateTableEntry(const std::string& path, sol::table&& tbl,
							   ParsedLuaFunctionTableSignatures&& fns)
{
	const auto id = tableIdCounter_++;

	return std::pair<TableId, ScriptTable2>{
		std::piecewise_construct,
			std::forward_as_tuple(id),
			std::forward_as_tuple(path, std::move(tbl), std::move(fns), id)
	};
}