#pragma once
#include "System.h"
#include "Observers.h"
#include "../scripting/LuaUserType.h"
#include "../components/ScriptComponent.h"
#include "../core/Result.h"
#include "../scripting/ScriptTable.h"
#include <unordered_map>
#include <filesystem>

struct ScriptDataDescriptor
{
	std::string filepath;
	ScriptTable::Descriptor tableDescriptor;
};

using ScriptDataPackage = std::vector<ScriptDataDescriptor>;

class ScriptSystem : public System
{
public:
	struct TableData
	{
		ScriptTable table;
		std::string filepath;
	};

	using TableDataMap = std::unordered_map<ScriptTable::TableId, TableData>;

	ScriptSystem() = default;
	~ScriptSystem();
	ScriptSystem(const ScriptSystem&) = delete;
	ScriptSystem& operator=(const ScriptSystem&) = delete;
	ScriptSystem(ScriptSystem&&) noexcept = delete;
	ScriptSystem& operator=(ScriptSystem&&) noexcept = delete;

	sol::state& GetState() { return state_; }

	template <SomeLuaUserType...Ts>
	void InitState();

	Result<ScriptTable::TableId> AddTable(const std::string& pathStr);
	Result<ScriptTable::TableId> AddTable(const std::filesystem::path& path);

	bool RegisterTableFunction(ScriptTable::TableId tableId, std::string_view fnName,
							   const ScriptSignature& scriptSig);

	template <HasFuncTraits Sig>
	bool RegisterTableFunction(ScriptTable::TableId tableId, std::string_view fnName);

	ScriptTable* GetTable(ScriptTable::TableId tableId);
	const ScriptTable* GetTable(ScriptTable::TableId tableId) const;

	ScriptTableView GetTableView(ScriptTable::TableId tableId) const; 

	const std::string& GetTableFilepath(ScriptTable::TableId tableId) const;

	bool RemoveTable(ScriptTable::TableId tableId);

	bool ContainsTable(ScriptTable::TableId tableId) const;

	Result<bool> ReloadTable(ScriptTable::TableId tableId);

	ScriptDataPackage ExportScriptDataPackage() const;

	const TableDataMap& GetTableDataMap() const { return tableData_; }

private:
	sol::state state_;
	TableDataMap tableData_;
};

template <SomeLuaUserType...Ts>
void ScriptSystem::InitState()
{
	state_.open_libraries(sol::lib::base);

	((LuaUserType<Ts>::Register(state_)), ...);
}

template <HasFuncTraits Sig>
bool ScriptSystem::RegisterTableFunction(ScriptTable::TableId tableId, std::string_view fnName)
{
	//if (auto it = tableIndexMap_.find(tableId); it != tableIndexMap_.end())
	//{
	//	assert(it->second < tables_.size());

	//	return tables_[it->second].RegisterFunction<Sig>(fnName);
	//}

	if (auto it = tableData_.find(tableId); it != tableData_.end())
	{
		return it->second.table.RegisterFunction<Sig>(fnName);
	}

	return false;
}