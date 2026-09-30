#pragma once
#include "ScriptableUserSubsystem.h"
#include "../scripting/LuaStateManager.h"
#include "../scripting/ScriptTable.h"
#include "../scripting/ScriptTableManager.h"
#include <unordered_map>
#include <filesystem>

class ScriptSystem : public System
{
public:
	using ScriptTableMap = std::unordered_map<ScriptTable::TableId, ScriptTableEntry>;

	ScriptSystem() = default;
	~ScriptSystem();
	ScriptSystem(const ScriptSystem&) = delete;
	ScriptSystem& operator=(const ScriptSystem&) = delete;
	ScriptSystem(ScriptSystem&&) noexcept = delete;
	ScriptSystem& operator=(ScriptSystem&&) noexcept = delete;

	Result<ScriptTable::TableId> AddTable(ScriptTableDescriptor&& descriptor);
	Result<Void> ReloadTable(ScriptTable::TableId tableId);
	bool RemoveTable(ScriptTable::TableId tableId);
	bool ContainsTable(ScriptTable::TableId tableId) const;
	ScriptTableView GetTableView(ScriptTable::TableId tableId) const;
	Result<Void> SetTableName(ScriptTable::TableId tableId, std::string_view newName);

	const ScriptTableManager& GetTableManager() const { return tables_; }

	LuaStateManager& GetState() { return state_; }
	const LuaStateManager& GetState() const { return state_; }

	void Reset();

	template <Phase ph>
	void Update(float dt)
	{
		scriptableUserSubsystem_.Update<ph>(dt);
	}

private:
	LuaStateManager state_;
	ScriptTableManager tables_;
	ScriptableUserSubsystem scriptableUserSubsystem_;
};