#pragma once
#include "ScriptTable.h"
#include "ScriptTableObserverSignal.h"
#include "LuaStateManager.h"


class ScriptTableManager
{
public:
	ScriptTableManager() = default;
	~ScriptTableManager() = default;
	ScriptTableManager(const ScriptTableManager&) = delete;
	ScriptTableManager& operator=(const ScriptTableManager&) = delete;
	ScriptTableManager(ScriptTableManager&&) noexcept = delete;
	ScriptTableManager& operator=(ScriptTableManager&&) noexcept = delete;

	Result<ScriptTable*> AddTable(const std::string& path, LuaStateManager& state,
										  ScriptTable::TableType type);
	Result<Void> ReloadTable(ScriptTable::TableId tableId, LuaStateManager& state);
	bool RemoveTable(ScriptTable::TableId tableId);
	bool ContainsTable(ScriptTable::TableId tableId) const;
	ScriptTableView GetTableView(ScriptTable::TableId tableId) const;
	const std::string& GetTableFilepath(ScriptTable::TableId tableId) const;
	ScriptTable::TableType GetTableType(ScriptTable::TableId tableId) const;
	void Clear();

	const ScriptTableDataMap& GetTableDataMap() const { return tableMap_; }

private:
	ScriptTableDataMap tableMap_;
};