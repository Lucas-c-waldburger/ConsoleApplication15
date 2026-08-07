#include "ScriptSystem.h"
#include "../ecs/Ecs.h"
#include "../components/util/ComponentValidPreds.h"
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

//std::vector<Entity> GetEntitiesToUpdate(ScriptTable::TableId removedTableId,
//										ScriptTable::TableId swappedTableId)
//{
//	return ECS::GetAllEntitiesWith<Script>([removedTableId, swappedTableId](const Script& script) {
//		return script.table.GetTableId() == removedTableId ||
//			   script.table.GetTableId() == swappedTableId;
//	});
//}

//void ReassignEntityScriptTables(ScriptTable::TableId )

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
	auto es = ECS::GetAllEntitiesWith<Script>(&ScriptValid);
	for (auto& e : es)
	{
		e.GetComponent<Script>().table = {};
	}
}

Result<ScriptTable::TableId> ScriptSystem::AddTable(const std::string& pathStr)
{
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

	auto newTable = ScriptTable::Create(loadResult.get<sol::table>());
	const auto id = newTable.GetTableId();

	tableData_.try_emplace(id, std::move(newTable), std::move(pathStr));

	return id;
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
		return MAKE_ERROR_FMT("File at path '{}' is not  a lua file", pathStr);
	}

	return AddTable(pathStr);
}

bool ScriptSystem::RegisterTableFunction(ScriptTable::TableId tableId, std::string_view fnName,
										 const ScriptSignature& scriptSig)
{
	if (auto it = tableData_.find(tableId); it != tableData_.end())
	{
		return it->second.table.RegisterFunction(fnName, scriptSig);
	}

	return false;
}

ScriptTable* ScriptSystem::GetTable(ScriptTable::TableId tableId)
{
	//if (auto it = tableIndexMap_.find(tableId); it != tableIndexMap_.end())
	//{
	//	assert(it->second < tables_.size());
	//	return &tables_[it->second];
	//}

	if (auto it = tableData_.find(tableId); it != tableData_.end())
	{
		return &it->second.table;
	}

	return nullptr;
}

const ScriptTable* ScriptSystem::GetTable(ScriptTable::TableId tableId) const
{
	//if (auto it = tableIndexMap_.find(tableId); it != tableIndexMap_.end())
	//{
	//	assert(it->second < tables_.size());
	//	return &tables_[it->second];
	//}

	if (auto it = tableData_.find(tableId); it != tableData_.end())
	{
		return &it->second.table;
	}

	return nullptr;
}

ScriptTableView ScriptSystem::GetTableView(ScriptTable::TableId tableId) const
{
	//if (auto it = tableIndexMap_.find(tableId); it != tableIndexMap_.end())
	//{
	//	assert(it->second < tables_.size());
	//	
	//	return tables_[it->second].GetView();
	//}

	if (auto it = tableData_.find(tableId); it != tableData_.end())
	{
		return it->second.table.GetView();
	}

	return {};
}

const std::string& ScriptSystem::GetTableFilepath(ScriptTable::TableId tableId) const
{
	//if (auto it = tableIndexMap_.find(tableId); it != tableIndexMap_.end())
	//{
	//	assert(it->second < filepaths_.size());
	//	return filepaths_[it->second];
	//}

	if (auto it = tableData_.find(tableId); it != tableData_.end())
	{
		return it->second.filepath;
	}

	return Null<std::string>();
}

bool ScriptSystem::RemoveTable(ScriptTable::TableId tableId)
{
	auto it = tableData_.find(tableId);
	if (it == tableData_.end())
	{
		return false;
	}

	InvalidateEntityScriptTables(tableId);

	tableData_.erase(it);

	return true;

	/*auto it = tableIndexMap_.find(tableId);
	if (it == tableIndexMap_.end())
	{
		return false;
	}

	assert(it->second < tables_.size());

	InvalidateEntityScriptTables(tableId);

	const size_t tableIdx = it->second;
	const size_t backIdx = tables_.size() - 1;

	if (tableIdx != backIdx)
	{
		const auto backTableId = tables_.back().GetTableId();

		auto es = ECS::GetAllEntitiesWith<Script>([backTableId](const Script& script) {
			return script.table.GetTableId() == backTableId;
		});

		std::swap(tables_[tableIdx], tables_[backIdx]);
		std::swap(filepaths_[tableIdx], filepaths_[backIdx]);
		tableIndexMap_[tables_[tableIdx].GetTableId()] = tableIdx;

		for (auto& e : es)
		{
			e.GetComponent<Script>().table = tables_[tableIdx].GetView();
		}
	}

	tables_.pop_back();
	filepaths_.pop_back();
	tableIndexMap_.erase(it);

	return true;*/
}

bool ScriptSystem::ContainsTable(ScriptTable::TableId tableId) const
{
	//return tableIndexMap_.contains(tableId);
	return tableData_.contains(tableId);
}

Result<bool> ScriptSystem::ReloadTable(ScriptTable::TableId tableId)
{
	auto it = tableData_.find(tableId);
	if (it == tableData_.end())
	{
		return false;
	}

	sol::protected_function_result loadResult = state_.data_.script_file(it->second.filepath);
	if (!loadResult.valid())
	{
		sol::error err = loadResult;
		return MAKE_ERROR(err.what());
	}

	if (loadResult.get_type() != sol::type::table)
	{
		return MAKE_ERROR_FMT("Script at path '{}' did not return a sol::table", it->second.filepath);
	}

	it->second.table.SetTable(loadResult.get<sol::table>());

	return true;
}

ScriptDataPackage ScriptSystem::ExportScriptDataPackage() const
{
	ScriptDataPackage package{};
	package.reserve(tableData_.size());

	for (const auto& [_, data] : tableData_)
	{
		package.emplace_back(data.filepath, data.table.ExportTableDescriptor());
	}

	return package;
}
