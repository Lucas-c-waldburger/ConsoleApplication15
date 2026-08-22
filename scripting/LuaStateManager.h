#pragma once
#include "LuaUserType.h"
#include "LuaNativeTypeIdUtils.h"
#include "../core/Dictionary.h"

class LuaStateManager
{
public:
	template <typename...Ts>
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

	sol::state_view Data() { return state_; }

private:
	sol::state state_;
	UnorderedDictionary<uint32_t> registeredNameToTypeId_;
};

namespace detail {
template <typename T>
struct register_lua_user_types_impl;

template <template <typename> class TList, SomeLuaUserType...Ts>
struct register_lua_user_types_impl<TList<Ts...>>
{
	static void call(sol::state_view state, UnorderedDictionary<uint32_t>& nameToTypeId)
	{
		((LuaUserType<Ts>::Register(state, nameToTypeId)), ...);
	}
};

template <SomeLuaUserType T>
struct register_lua_user_types_impl<T>
{
	static void call(sol::state_view state, UnorderedDictionary<uint32_t>& nameToTypeId)
	{
		LuaUserType<T>::Register(state, nameToTypeId);
	}
};

}

template <typename...Ts>
inline void LuaStateManager::InitWithEngineTypes()
{
	state_.open_libraries(sol::lib::base);

	((detail::register_lua_user_types_impl<Ts>::call(state_, registeredNameToTypeId_)), ...);

	//((LuaUserType<Ts>::Register(state_, registeredNameToTypeId_)), ...);

	//static constexpr auto registerType = []<typename T>(UnorderedDictionary<uint32_t>& map) {
	//	map.try_emplace(lua_user_type_name<T>::value, TypeInfo<T>::hash32);
	//};

	//((registerType.template operator()<Ts>(registeredNameToTypeId_)), ...);
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
