#pragma once
#include "ScriptableUserSubsystem.h"
#include "../scripting/LuaStateManager.h"
#include "../scripting/ScriptTable.h"
#include "../scripting/ScriptTableObserverSignal.h"
#include "../scripting/ScriptTableManager.h"
#include <unordered_map>
#include <filesystem>

struct ScriptTableDescriptors
{
	std::vector<std::string> filepaths;
	std::vector<std::vector<std::string>> functionNames;
};

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

	Result<ScriptTable::TableId> AddFunctionTable(const std::string& path);
	Result<ScriptTable::TableId> AddSystemTable(const std::string& path, Phase phase);

	Result<Void> ReloadTable(ScriptTable::TableId tableId);
	bool RemoveTable(ScriptTable::TableId tableId);
	bool ContainsTable(ScriptTable::TableId tableId) const;
	ScriptTableView GetTableView(ScriptTable::TableId tableId) const;
	const std::string& GetTableFilepath(ScriptTable::TableId tableId) const;
	const ScriptTableDataMap& GetScriptTableMap() const;

	//ScriptTableDescriptors ExportTableDescriptors() const;

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