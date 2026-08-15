#pragma once
#include "LuaUserType.h"
#include "LuaNativeTypeIdUtils.h"
#include "../core/Dictionary.h"

class LuaStateManager
{
public:
	template <SomeLuaUserType...Ts>
	void InitWithEngineTypes();

	template <typename T, typename...Args>
		requires (std::same_as<raw_type_t<T>, T> && std::is_class_v<T>)
	bool NewUserType(std::string_view name, Args&&...args);

	template <typename T, typename...Args>
		requires (std::same_as<raw_type_t<T>, T> && std::is_enum_v<T>)
	bool NewEnum(std::string_view name, Args&&...args);

	sol::protected_function_result LoadScriptFile(const std::string& path);

	sol::protected_function_result LoadScriptString(const std::string& str);

	bool IsRegistered(std::string_view name) const;

	uint32_t GetRegisteredTypeId(std::string_view name) const;

	lua_State* Data() { return state_.lua_state(); }

private:
	sol::state state_;
	UnorderedDictionary<uint32_t> registeredNameToTypeId_;
};

template <SomeLuaUserType ...Ts>
inline void LuaStateManager::InitWithEngineTypes()
{
	state_.open_libraries(sol::lib::base);

	((LuaUserType<Ts>::Register(state_)), ...);
}

template <typename T, typename ...Args>
	requires (std::same_as<raw_type_t<T>, T>&& std::is_class_v<T>)
inline bool LuaStateManager::NewUserType(std::string_view name, Args && ...args)
{
	using Type = raw_type_t<T>;

	if (IsRegistered(name))
	{
		return false;
	}

	state_.new_usertype<Type>(name, std::forward<Args>(args)...);

	registeredNameToTypeId_.try_emplace(name, TypeInfo<Type>::hash32);

	return true;
}

template <typename T, typename ...Args>
	requires (std::same_as<raw_type_t<T>, T>&& std::is_enum_v<T>)
inline bool LuaStateManager::NewEnum(std::string_view name, Args && ...args)
{
	using Type = raw_type_t<T>;

	if (IsRegistered(name))
	{
		return false;
	}

	state_.new_enum(name, std::forward<Args>(args)...);

	registeredNameToTypeId_.try_emplace(name, TypeInfo<Type>::hash32);

	return true;
}
