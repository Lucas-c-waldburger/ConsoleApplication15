#pragma once
#include "System.h"
#include "Observers.h"
#include "../scripting/LuaUserType.h"
#include "../components/ScriptComponent.h"
#include "../core/Result.h"
#include <unordered_map>
#include <filesystem>

class ScriptSystem : public System, 
					 public EntityDestroyedObserver<ScriptSystem>,
					 public ComponentRemovedObserver<ScriptSystem>
{
public:
	friend class EntityDestroyedObserver<ScriptSystem>;
	friend class ComponentRemovedObserver<ScriptSystem>;

	ScriptSystem();

	template <SomeLuaUserType...Ts>
	Result<Script> RegisterScript(const std::filesystem::path& path);

private:
	void OnEntityDestroyed(Entity e);
	void OnComponentRemoved(Entity e, ComponentSignature sig);

	std::unordered_map<Handle<LuaState>, sol::state> stateMap_;
};

template <SomeLuaUserType...Ts>
Result<Script> ScriptSystem::RegisterScript(const std::filesystem::path& path)
{
	auto pathStr = path.string();

	if (!std::filesystem::exists(path))
	{
		return MAKE_ERROR_FMT("File at path '{}' does not exist", pathStr);
	}
	if (path.stem() != ".lua")
	{
		return MAKE_ERROR_FMT("File at path '{}' is not a lua file", pathStr);
	}

	auto [it, _] = stateMap_.try_emplace(Handle<LuaState>::Create(), sol::state{});
	auto& [handle, state] = *it;

	state.open_libraries(sol::lib::base);

	((LuaUserType<Ts>::Register(state)), ...);

	sol::protected_function_result loadResult = state.script_file(pathStr);
	if (!loadResult.valid())
	{
		stateMap_.erase(it);

		sol::error err = loadResult;
		return MAKE_ERROR(err.what());
	}

	if (!loadResult.get_type() != sol::type::table)
	{
		stateMap_.erase(it);

		return MAKE_ERROR_FMT("Script at path '{}' did not return a sol::table", pathStr);
	}

	return Script{
		.stateHandle = handle,
		.table = loadResult
	};
}
