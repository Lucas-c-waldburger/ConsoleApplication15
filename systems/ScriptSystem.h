#pragma once
#include "System.h"
#include "Observers.h"
#include "../scripting/LuaUserType.h"
#include "../components/ScriptComponent.h"
#include "../core/Result.h"
#include "../scripting/ScriptTable.h"
#include <unordered_map>
#include <filesystem>

class ScriptSystem : public System
{
public:
	class State
	{
	public:
		~State() = default;
		State(const State&) = delete;
		State& operator=(const State&) = delete;
		State(State&&) noexcept = delete;
		State& operator=(State&&) noexcept = delete;

		template <SomeLuaUserType...Ts>
		void Init();

		template <typename Class, typename...Args>
		decltype(auto) AddUserType(Args&&...args)
		{
			return data_.new_usertype<Class>(std::forward<Args>(args)...);
		}

		template <typename...Args>
		decltype(auto) AddEnum(std::string_view name, Args&&...args)
		{
			return data_.new_enum(name, std::forward<Args>(args)...);
		}

	private:
		friend class ScriptSystem;

		State() = default;

		sol::state data_;
	};

	ScriptSystem() = default;
	~ScriptSystem();
	ScriptSystem(const ScriptSystem&) = delete;
	ScriptSystem& operator=(const ScriptSystem&) = delete;
	ScriptSystem(ScriptSystem&&) noexcept = delete;
	ScriptSystem& operator=(ScriptSystem&&) noexcept = delete;

	State& GetState() { return state_; }

	template <SomeLuaUserType...Ts>
	void InitState();

	Result<ScriptTable::TableId> AddTable(const std::filesystem::path& path);

	template <HasFuncTraits Sig>
	bool RegisterTableFunction(ScriptTable::TableId tableId, std::string_view fnName);

	ScriptTable* GetTable(ScriptTable::TableId tableId);
	const ScriptTable* GetTable(ScriptTable::TableId tableId) const;

	ScriptTableView GetTableView(ScriptTable::TableId tableId) const; 

	const std::string& GetTableFilepath(ScriptTable::TableId tableId) const;

	bool RemoveTable(ScriptTable::TableId tableId);

	bool ContainsTable(ScriptTable::TableId tableId) const;

private:
	State state_;

	std::vector<std::string> filepaths_;
	std::vector<ScriptTable> tables_;
	std::unordered_map<ScriptTable::TableId, size_t> tableIndexMap_;
};

template <SomeLuaUserType...Ts>
void ScriptSystem::State::Init()
{
	data_.open_libraries(sol::lib::base);

	((LuaUserType<Ts>::Register(data_)), ...);
}

template <SomeLuaUserType...Ts>
void ScriptSystem::InitState()
{
	state_.data_.open_libraries(sol::lib::base);

	((LuaUserType<Ts>::Register(state_.data_)), ...);
}

template <HasFuncTraits Sig>
bool ScriptSystem::RegisterTableFunction(ScriptTable::TableId tableId, std::string_view fnName)
{
	if (auto it = tableIndexMap_.find(tableId); it != tableIndexMap_.end())
	{
		assert(it->second < tables_.size());

		return tables_[it->second].RegisterFunction<Sig>(fnName);
	}

	return false;
}