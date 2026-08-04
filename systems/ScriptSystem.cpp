#include "ScriptSystem.h"
#include "../ecs/Ecs.h"
#include <ranges>

namespace {

void InvalidateEntityScriptTables(ScriptTable::TableId tableId)
{
	auto es = ECS::GetAllEntitiesWith<Script>([tableId](const Script& script) {
		return script.table.GetTableId() == tableId;
	});

	for (auto& e : es)
	{
		e.GetComponent<Script>().table = {};
	}
}

void EraseScriptCallbacks(Entity& e)
{
	if (!e.HasComponent<SignalTokenStorage>())
	{
		return;
	}

	auto& tks = e.GetComponent<SignalTokenStorage>().signalTokens;

	core::EraseIf(tks, [](const auto& tk) {
		return tk.type == EntityCallbackToken::Type::Script; }
	);
}

} // unnamed

ScriptSystem::~ScriptSystem()
{
	for (auto& table : tables_)
	{
		InvalidateEntityScriptTables(table.GetTableId());
	}
}

Result<ScriptTable::TableId> ScriptSystem::AddTable(const std::filesystem::path& path)
{
	auto pathStr = path.string();

	if (!std::filesystem::exists(path))
	{
		return MAKE_ERROR_FMT("File at path '{}' does not exist", pathStr);
	}
	if (path.extension() != ".lua")
	{
		return MAKE_ERROR_FMT("File at path '{}' is not a lua file", pathStr);
	}

	sol::protected_function_result loadResult = state_.data_.script_file(pathStr);
	if (!loadResult.valid())
	{
		sol::error err = loadResult;
		return MAKE_ERROR(err.what());
	}

	if (loadResult.get_type() != sol::type::table)
	{
		return MAKE_ERROR_FMT("Script at path '{}' did not return a sol::table", pathStr);
	}

	const size_t idx = tables_.size();

	filepaths_.emplace_back(std::move(pathStr));

	auto& newTable = tables_.emplace_back(ScriptTable::Create(loadResult.get<sol::table>()));
	const auto id = newTable.GetTableId();

	tableIndexMap_.emplace(id, idx);

	return id;
}

ScriptTable* ScriptSystem::GetTable(ScriptTable::TableId tableId)
{
	if (auto it = tableIndexMap_.find(tableId); it != tableIndexMap_.end())
	{
		assert(it->second < tables_.size());
		return &tables_[it->second];
	}

	return nullptr;
}

const ScriptTable* ScriptSystem::GetTable(ScriptTable::TableId tableId) const
{
	if (auto it = tableIndexMap_.find(tableId); it != tableIndexMap_.end())
	{
		assert(it->second < tables_.size());
		return &tables_[it->second];
	}

	return nullptr;
}

ScriptTableView ScriptSystem::GetTableView(ScriptTable::TableId tableId) const
{
	if (auto it = tableIndexMap_.find(tableId); it != tableIndexMap_.end())
	{
		assert(it->second < tables_.size());
		
		return tables_[it->second].GetView();
	}

	return {};
}

const std::string& ScriptSystem::GetTableFilepath(ScriptTable::TableId tableId) const
{
	if (auto it = tableIndexMap_.find(tableId); it != tableIndexMap_.end())
	{
		assert(it->second < filepaths_.size());
		return filepaths_[it->second];
	}

	return Null<std::string>();
}

bool ScriptSystem::RemoveTable(ScriptTable::TableId tableId)
{
	auto it = tableIndexMap_.find(tableId);
	if (it == tableIndexMap_.end())
	{
		return false;
	}

	assert(it->second < tables_.size());

	const size_t backIdx = tables_.size() - 1;
	if (it->second != backIdx)
	{
		std::swap(tables_[it->second], tables_[backIdx]);
		std::swap(filepaths_[it->second], filepaths_[backIdx]);
		tableIndexMap_[tables_[it->second].GetTableId()] = it->second;
	}

	InvalidateEntityScriptTables(tableId);

	tables_.pop_back();
	filepaths_.pop_back();
	tableIndexMap_.erase(it);

	return true;
}

bool ScriptSystem::ContainsTable(ScriptTable::TableId tableId) const
{
	return tableIndexMap_.contains(tableId);
}
