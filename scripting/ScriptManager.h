#pragma once
#include "Lua.h"
#include <unordered_map>
#include <functional>

struct ScriptInstance
{
	using Setup = std::function<void(Lua&)>;
	ScriptInfo scriptInfo;
	Setup setupFn;
};

class ScriptManager
{
public:
	template <typename...Ts, typename...Libs>
	void RegisterScript(ScriptInstance instance, Libs&&...libs)
	{
		auto& [info, setup] = instance;

		if (scriptMap_.contains(info.name))
		{
			LOG_WARNING("Overwriting script with name: \"", info.name, '"');
		}

		auto& lua = scriptMap_[info.name];

		lua = Lua::GetInstance<Ts...>(std::forward<Libs>(libs)...);

		lua.SetScriptInfo(std::move(info));

		if (setup)
		{
			setup(lua);
		}
	}

	void RegisterScript(Lua&& lua)
	{
		const std::string& scriptName = lua.GetScriptInfo().name;

		if (scriptMap_.contains(scriptName))
		{
			LOG_WARNING("Overwriting script with name: \"", scriptName, '"');
		}

		scriptMap_[scriptName] = std::move(lua);
	}

	bool RemoveScript(const std::string& scriptName)
	{
		return scriptMap_.erase(scriptName) > 0;
	}

	Result<Void> RunScript(const std::string& scriptName)
	{
		auto it = scriptMap_.find(scriptName);
		if (it == scriptMap_.end())
		{
			LOG_WARNING("Script named \"", scriptName, "\" not found");
			return Void{};
		}

		return it->second.Run();
	}

private:
	std::unordered_map<std::string, Lua> scriptMap_;
};

