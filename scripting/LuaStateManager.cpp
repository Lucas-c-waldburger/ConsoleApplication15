#include "LuaStateManager.h"

void LuaStateManager::InitWithEngineTypes()
{
	state_.open_libraries(sol::lib::base);

	RegisterLuaUserTypes(*this);
}

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

	if (GetLuaUserTypeInfo().GetDataIndex(name).IsValid())
	{
		return state_[name] != sol::lua_nil;
	}

	return false;
}

uint32_t LuaStateManager::GetRegisteredTypeId(std::string_view name) const
{
	if (IsNativeLuaType(name))
	{
		return GetNativeLuaTypeId(name);
	}

	if (auto idx = GetLuaUserTypeInfo().GetDataIndex(name); idx.IsValid())
	{
		return userTypeInfo_.GetUserTypeIds()[idx];
	}

	return kInvalidLuaTypeId;
}

const ParsedLuaUserTypeInfo& LuaStateManager::GetLuaUserTypeInfo() const
{
	if (!userTypeInfo_.IsCommitted())
	{
		userTypeInfo_.Commit();
	}

	return userTypeInfo_;
}
