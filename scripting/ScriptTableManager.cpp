#include "ScriptTableManager.h"

Result<ScriptTable*> 
ScriptTableManager::AddTable(const std::string& path, LuaStateManager& state, ScriptTable::TableType type)
{
	auto loadResult = state.LoadScriptFile(path);
	if (!loadResult.valid())
	{
		sol::error err = loadResult;
		return MAKE_ERROR(err.what());
	}

	if (loadResult.get_type() != sol::type::table)
	{
		return MAKE_ERROR_FMT("Script at path '{}' did not return a sol::table", path);
	}

	auto table = loadResult.get<sol::table>();

	TRY(LuaFunctionTableParser::ParseLuaFunctionTableSignatures(table, state), parsed);

	const auto it = tableMap_.emplace(
		ScriptTable::CreateTableEntry(path, std::move(table), std::move(parsed), type)).first;

	return &it->second.table;
}

Result<Void> ScriptTableManager::ReloadTable(ScriptTable::TableId tableId, LuaStateManager& state)
{
	auto it = tableMap_.find(tableId);
	if (it == tableMap_.end())
	{
		return MAKE_ERROR("Table with provided Id not found");
	}

	auto loadResult = state.LoadScriptFile(it->second.filepath);
	if (!loadResult.valid())
	{
		sol::error err = loadResult;
		return MAKE_ERROR(err.what());
	}

	if (loadResult.get_type() != sol::type::table)
	{
		return MAKE_ERROR_FMT("Script at path '{}' did not return a sol::table",
			it->second.filepath);
	}

	auto table = loadResult.get<sol::table>();

	TRY(LuaFunctionTableParser::ParseLuaFunctionTableSignatures(table, state), parsed);

	it->second.table.ReassignTableData(std::move(table), std::move(parsed));

	return kVoid;
}

bool ScriptTableManager::RemoveTable(ScriptTable::TableId tableId)
{
	auto it = tableMap_.find(tableId);
	if (it == tableMap_.end())
	{
		return false;
	}

	tableMap_.erase(it);

	return true;
}

bool ScriptTableManager::ContainsTable(ScriptTable::TableId tableId) const
{
	return tableMap_.contains(tableId);
}

ScriptTableView ScriptTableManager::GetTableView(ScriptTable::TableId tableId) const
{
	if (auto it = tableMap_.find(tableId); it != tableMap_.end())
	{
		return ScriptTableView{ &it->second.table };
	}

	return ScriptTableView{};
}

const std::string& ScriptTableManager::GetTableFilepath(ScriptTable::TableId tableId) const
{
	if (auto it = tableMap_.find(tableId); it != tableMap_.end())
	{
		return it->second.filepath;
	}

	return Null<std::string>();
}

ScriptTable::TableType ScriptTableManager::GetTableType(ScriptTable::TableId tableId) const
{
	if (auto it = tableMap_.find(tableId); it != tableMap_.end())
	{
		return it->second.type;
	}

	return ScriptTable::TableType::Unknown;
}

void ScriptTableManager::Clear() 
{ 
	tableMap_.clear(); 
}
