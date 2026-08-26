#pragma once
#include "LuaUserType.h"
#include "LuaNativeTypeIdUtils.h"
#include "../core/Dictionary.h"
#include "../core/Reflection.h"

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

	template <typename T> 
		requires (std::same_as<raw_type_t<T>, T> && 
				 ((std::is_class_v<T> && PfrReflectable<T>) || std::is_enum_v<T>))
	bool AutoRegister(std::string_view name);

	sol::protected_function_result LoadScriptFile(const std::string& path);

	sol::protected_function_result LoadScriptString(const std::string& str);

	bool IsRegistered(std::string_view name) const;

	//template <typename T> requires std::same_as<raw_type_t<T>, T>
	//bool IsRegistered() const;

	uint32_t GetRegisteredTypeId(std::string_view name) const;

	//template <typename T> requires std::same_as<raw_type_t<T>, T>
	//const std::string& GetRegisteredName() const;

	sol::state_view Data() { return state_; }

private:
	sol::state state_;
	UnorderedDictionary<uint32_t> registeredNameToTypeId_;
	//std::unordered_map<uint32_t, std::string> registeredTypeIdToName_;
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
}

template <typename T, typename ...Args>
	requires (std::same_as<raw_type_t<T>, T>&& std::is_class_v<T>)
inline bool LuaStateManager::NewUserType(std::string_view name, Args && ...args)
{
	if (IsRegistered(name))
	{
		return false;
	}

	state_.new_usertype<T>(name, std::forward<Args>(args)...);

	registeredNameToTypeId_.try_emplace(name, TypeInfo<T>::hash32);
	//registeredTypeIdToName_.try_emplace(TypeInfo<T>::hash32, name);

	return true;
}

template <typename T, typename ...Args>
	requires (std::same_as<raw_type_t<T>, T> && std::is_enum_v<T>)
inline bool LuaStateManager::NewEnum(std::string_view name, Args && ...args)
{
	if (IsRegistered(name))
	{
		return false;
	}

	state_.new_enum(name, std::forward<Args>(args)...);

	registeredNameToTypeId_.try_emplace(name, TypeInfo<T>::hash32);
	//registeredTypeIdToName_.try_emplace(TypeInfo<T>::hash32, name);

	return true;
}

//template <typename T> requires std::same_as<raw_type_t<T>, T>
//bool LuaStateManager::IsRegistered() const
//{
//	return registeredTypeIdToName_.contains(TypeInfo<T>::hash32);
//}
//
//template <typename T> requires std::same_as<raw_type_t<T>, T>
//const std::string& LuaStateManager::GetRegisteredName() const
//{
//	auto it = registeredTypeIdToName_.find(TypeInfo<T>::hash32);
//
//	return (it != registeredTypeIdToName_.end()) ? it->second : Null<std::string>();
//}

template <typename T>
	requires (std::same_as<raw_type_t<T>, T> &&
			 ((std::is_class_v<T> && PfrReflectable<T>) || std::is_enum_v<T>))
bool LuaStateManager::AutoRegister(std::string_view name)
{
	if (IsRegistered(name))
	{
		return false;
	}

	if constexpr (std::is_enum_v<T>)
	{
		AutoRegisterEnum<T>(state_, name);
	}
	else
	{
		AutoRegisterUserType<T>(state_, name);
	}

	registeredNameToTypeId_.try_emplace(name, TypeInfo<T>::hash32);
	//registeredTypeIdToName_.try_emplace(TypeInfo<T>::hash32, name);

	return true;
}