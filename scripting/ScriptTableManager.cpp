#include "ScriptTableManager.h"
#include <filesystem>
#include <ranges>

//Result<ScriptTable*> 
//ScriptTableManager::AddTable(const std::string& path, LuaStateManager& state, ScriptTable::TableType type)
//{
//	auto loadResult = state.LoadScriptFile(path);
//	if (!loadResult.valid())
//	{
//		sol::error err = loadResult;
//		return MAKE_ERROR(err.what());
//	}
//
//	if (loadResult.get_type() != sol::type::table)
//	{
//		return MAKE_ERROR_FMT("Script at path '{}' did not return a sol::table", path);
//	}
//
//	auto table = loadResult.get<sol::table>();
//
//	TRY(LuaFunctionTableParser::ParseLuaFunctionTableSignatures(table, state), parsed);
//
//	const auto it = tableMap_.emplace(
//		ScriptTable::CreateTableEntry(std::filesystem::path(path).filename().string(),
//			std::move(path), std::move(table), std::move(parsed), type)).first;
//
//	return &it->second.table;
//}
//
//Result<Void> ScriptTableManager::ReloadTable(ScriptTable::TableId tableId, LuaStateManager& state)
//{
//	auto it = tableMap_.find(tableId);
//	if (it == tableMap_.end())
//	{
//		return MAKE_ERROR("Table with provided Id not found");
//	}
//
//	auto loadResult = state.LoadScriptFile(it->second.filepath);
//	if (!loadResult.valid())
//	{
//		sol::error err = loadResult;
//		return MAKE_ERROR(err.what());
//	}
//
//	if (loadResult.get_type() != sol::type::table)
//	{
//		return MAKE_ERROR_FMT("Script at path '{}' did not return a sol::table",
//			it->second.filepath);
//	}
//
//	auto table = loadResult.get<sol::table>();
//
//	TRY(LuaFunctionTableParser::ParseLuaFunctionTableSignatures(table, state), parsed);
//
//	it->second.table.ReassignTableData(std::move(table), std::move(parsed));
//
//	return kVoid;
//}
//
//bool ScriptTableManager::RemoveTable(ScriptTable::TableId tableId)
//{
//	auto it = tableMap_.find(tableId);
//	if (it == tableMap_.end())
//	{
//		return false;
//	}
//
//	tableMap_.erase(it);
//
//	return true;
//}
//
//bool ScriptTableManager::ContainsTable(ScriptTable::TableId tableId) const
//{
//	return tableMap_.contains(tableId);
//}
//
//ScriptTableView ScriptTableManager::GetTableView(ScriptTable::TableId tableId) const
//{
//	if (auto it = tableMap_.find(tableId); it != tableMap_.end())
//	{
//		return ScriptTableView{ &it->second.table };
//	}
//
//	return ScriptTableView{};
//}
//
//const std::string& ScriptTableManager::GetTableName(ScriptTable::TableId tableId) const
//{
//	if (auto it = tableMap_.find(tableId); it != tableMap_.end())
//	{
//		return it->second.name;
//	}
//
//	return Null<std::string>();
//}
//
//const std::string& ScriptTableManager::GetTableFilepath(ScriptTable::TableId tableId) const
//{
//	if (auto it = tableMap_.find(tableId); it != tableMap_.end())
//	{
//		return it->second.filepath;
//	}
//
//	return Null<std::string>();
//}
//
//ScriptTable::TableType ScriptTableManager::GetTableType(ScriptTable::TableId tableId) const
//{
//	if (auto it = tableMap_.find(tableId); it != tableMap_.end())
//	{
//		return it->second.type;
//	}
//
//	return ScriptTable::TableType::Invalid;
//}
//
//void ScriptTableManager::Clear()  
//{ 
//	tableMap_.clear(); 
//}
//
//bool ScriptTableManager::SetTableName(ScriptTable::TableId tableId, std::string_view newName)
//{
//	if (auto it = tableMap_.find(tableId); it != tableMap_.end())
//	{
//		it->second.name = newName;
//
//		return true;
//	}
//
//	return false;
//}

Result<ScriptTable::TableId> 
ScriptTableManager::AddTable(ScriptTableDescriptor&& descriptor, LuaStateManager& state)
{
	auto path = std::filesystem::path(descriptor.filepath);

	if (!std::filesystem::exists(path))
	{
		return MAKE_ERROR_FMT("Invalid filepath: '{}'", descriptor.filepath);
	}

	if (descriptor.name.empty())
	{
		descriptor.name = path.stem().string();
	}

	auto loadResult = state.LoadScriptFile(descriptor.filepath);
	if (!loadResult.valid())
	{
		sol::error err = loadResult;
		return MAKE_ERROR(err.what());
	}

	if (loadResult.get_type() != sol::type::table)
	{
		return MAKE_ERROR_FMT("Script at path '{}' did not return a sol::table", 
			path.string());
	}

	auto table = loadResult.get<sol::table>();

	TRY(LuaFunctionTableParser::ParseLuaFunctionTableSignatures(table, state), parsed);

	if (!freeSlots_.empty())
	{
		assert(freeSlots_.back() < tableInfo_.Size());

		auto [it, _] = tables_.emplace(ScriptTable::CreateTableIndexPair(
			std::move(table), std::move(parsed), freeSlots_.back()));

		auto [nm, fp, type, sysPhase, tblId] = tableInfo_.GetView(freeSlots_.back());

		assert(nm.empty());
		assert(fp.empty());

		nm = std::move(descriptor.name);
		fp = std::move(descriptor.filepath);
		type = descriptor.tableType;
		sysPhase = descriptor.systemPhase;
		tblId = it->first;

		freeSlots_.pop_back();

		return it->first;
	}

	const size_t infoIdx = tableInfo_.PushBack(ScriptTableInfo{
		.name = std::move(descriptor.name),
		.filepath = std::move(descriptor.filepath),
		.tableType = descriptor.tableType,
		.systemPhase = descriptor.systemPhase
	});

	auto [it, _] = tables_.emplace(ScriptTable::CreateTableIndexPair(
		std::move(table), std::move(parsed), infoIdx));

	tableInfo_.GetView<&ScriptTableInfo::tableId>(infoIdx) = it->first;

	return it->first;
}

Result<Void> ScriptTableManager::ReloadTable(ScriptTable::TableId tableId, LuaStateManager& state)
{
	auto it = tables_.find(tableId);
	if (it == tables_.end())
	{
		return MAKE_ERROR_FMT("Script table with Id '{}' not found", tableId);
	}

	assert(it->second.resourceIndex < tableInfo_.Size());

	const auto& fp = tableInfo_.GetView<&ScriptTableInfo::filepath>(it->second.resourceIndex);

	assert(!fp.empty());

	auto loadResult = state.LoadScriptFile(fp);
	if (!loadResult.valid())
	{
		sol::error err = loadResult;
		return MAKE_ERROR(err.what());
	}

	if (loadResult.get_type() != sol::type::table)
	{
		return MAKE_ERROR_FMT("Script at path '{}' did not return a sol::table", fp);
	}

	auto table = loadResult.get<sol::table>();

	TRY(LuaFunctionTableParser::ParseLuaFunctionTableSignatures(table, state), parsed);

	it->second.table.ReassignTableData(std::move(table), std::move(parsed));

	return kVoid;
}

bool ScriptTableManager::RemoveTable(ScriptTable::TableId tableId)
{
	auto it = tables_.find(tableId);
	if (it == tables_.end())
	{
		return false;
	}

	const auto resourceIdx = it->second.resourceIndex;

	assert(resourceIdx < tableInfo_.Size());

	auto [nm, fp, type, sysPhase, tblId] = tableInfo_.GetView(resourceIdx);

	assert(!nm.empty());
	assert(!fp.empty());

	nm.clear();
	fp.clear();
	type = ScriptTable::TableType::Invalid;
	sysPhase = Phase::Invalid;
	tblId = ScriptTable::kInvalidTableId;

	tables_.erase(it);

	freeSlots_.emplace_back(resourceIdx);

	return true;
}

ScriptTable* ScriptTableManager::GetTable(ScriptTable::TableId tableId)
{
	auto it = tables_.find(tableId);
	if (it == tables_.end())
	{
		return nullptr;
	}

	return &it->second.table;
}

ScriptTableView ScriptTableManager::GetTableView(ScriptTable::TableId tableId) const
{
	auto it = tables_.find(tableId);
	if (it == tables_.end())
	{
		return {};
	}

	return it->second.table.GetView();
}

ScriptTableDescriptors ScriptTableManager::Serialize() const
{
	ScriptTableDescriptors descriptors;
	descriptors.reserve(tableInfo_.Size());

	for (size_t i = 0; i < tableInfo_.Size(); ++i)
	{
		auto [nm, fp, tableType, sysPhase, _] = tableInfo_.GetView(i);

		if (!fp.empty())
		{
			descriptors.emplace_back(nm, fp, tableType, sysPhase);
		}
	}

	return descriptors;
}

ScriptTable::TableType
ScriptTableManager::ResolveTableType(const ParsedLuaFunctionTableSignatures& fnSigs, 
									  ScriptTable::TableType reportedTableType)
{
	if (reportedTableType != ScriptTable::TableType::SystemTable)
	{
		return reportedTableType;
	}

	// check system table
	if (auto updateIt = fnSigs.find("update"); updateIt != fnSigs.end())
	{
		const bool validArgs = updateIt->second.empty() || (updateIt->second.size() == 1 &&
			updateIt->second.front() == GetNativeLuaTypeId("number"));

		if (validArgs)
		{
			if (auto initIt = fnSigs.find("init"); initIt != fnSigs.end())
			{
				if (!initIt->second.empty())
				{
					return ScriptTable::TableType::FunctionTable;
				}
			}

			return ScriptTable::TableType::SystemTable;
		}
	}

	return ScriptTable::TableType::FunctionTable;
}

bool ScriptTableManager::ContainsTable(ScriptTable::TableId tableId) const
{
	return tables_.contains(tableId);
}

void ScriptTableManager::Clear()
{
	tableInfo_.Clear();
	tables_.clear();
	freeSlots_.clear();
}

Result<Void> ScriptTableManager::SetTableName(ScriptTable::TableId tableId, std::string_view newName)
{
	if (newName.empty())
	{
		return MAKE_ERROR("New table name cannot be empty");
	}

	auto it = tables_.find(tableId);
	if (it == tables_.end())
	{
		return MAKE_ERROR_FMT("Table with ID '{}' not found", tableId);
	}

	assert(it->second.resourceIndex < tableInfo_.Size());

	auto& nm = tableInfo_.GetView<&ScriptTableInfo::name>(it->second.resourceIndex);

	nm = newName;

	return kVoid;
}