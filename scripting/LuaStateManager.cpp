#include "LuaStateManager.h"

sol::protected_function_result LuaStateManager::LoadScriptFile(const std::string& path)
{
	return state_.script_file(path);
}

sol::protected_function_result LuaStateManager::LoadScriptString(const std::string& str)
{
	return state_.script(str);
}

bool LuaStateManager::IsRegistered(std::string_view name) const
{
	if (IsNativeLuaType(name))
	{
		return true;
	}

	return state_[name] != sol::type::nil && registeredNameToTypeId_.contains(name);
}

uint32_t LuaStateManager::GetRegisteredTypeId(std::string_view name) const
{
	auto it = registeredNameToTypeId_.find(name);

	return (it != registeredNameToTypeId_.end()) ? it->second : kInvalidLuaTypeId;
}