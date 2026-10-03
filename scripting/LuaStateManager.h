#pragma once
#include "register/RegisterLuaUserTypes.h"
#include "LuaNativeTypeIdUtils.h"
#include "ParsedLuaUserTypeInfo.h"
#include "../core/Reflection.h"

class LuaStateManager
{
public:
	LuaStateManager() = default;
	~LuaStateManager() = default;
	LuaStateManager(const LuaStateManager&) = delete;
	LuaStateManager& operator=(const LuaStateManager&) = delete;
	LuaStateManager(LuaStateManager&&) noexcept = default;
	LuaStateManager& operator=(LuaStateManager&&) noexcept = default;

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

	template <typename T>
		requires (std::same_as<raw_type_t<T>, T> &&
				 (std::is_class_v<T> || std::is_enum_v<T>))
	bool IsRegistered() const;

	uint32_t GetRegisteredTypeId(std::string_view name) const;

	template <typename T>
		requires (std::same_as<raw_type_t<T>, T> &&
				 (std::is_class_v<T> || std::is_enum_v<T>))
	std::string_view GetRegisteredName() const;

	sol::state_view Data() { return state_; }

	const ParsedLuaUserTypeInfo& GetLuaUserTypeInfo() const;

private:
	mutable ParsedLuaUserTypeInfo userTypeInfo_;
	sol::state state_;
};

template <typename T, typename...Args>
	requires (std::same_as<raw_type_t<T>, T>&& std::is_class_v<T>)
inline bool LuaStateManager::NewUserType(std::string_view name, Args&&...args)
{
	if (IsRegistered(name))
	{
		return false;
	}

	userTypeInfo_.ParseUserTypeArgs<T>(name, args...);

	state_.new_usertype<T>(name, std::forward<Args>(args)...);

	return true;
}

template <typename T, typename ...Args>
	requires (std::same_as<raw_type_t<T>, T> && std::is_enum_v<T>)
inline bool LuaStateManager::NewEnum(std::string_view name, Args&& ...args)
{
	if (IsRegistered(name))
	{
		return false;
	}

	userTypeInfo_.ParseUserTypeArgs<T>(name, args...);

	state_.new_enum(name, std::forward<Args>(args)...);

	return true;
}

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
		AutoRegisterEnum<T>(state_, userTypeInfo_, name);
	}
	else
	{
		AutoRegisterUserType<T>(state_, userTypeInfo_, name);
	}

	return true;
}

template <typename T>
	requires (std::same_as<raw_type_t<T>, T> &&
			 (std::is_class_v<T> || std::is_enum_v<T>))
bool LuaStateManager::IsRegistered() const
{
	if (IsNativeLuaType<T>())
	{
		return true;
	}

	if (auto idx = GetLuaUserTypeInfo().GetDataIndex<T>(); idx.IsValid())
	{
		return state_[userTypeInfo_.GetUserTypeNames()[idx]] != sol::lua_nil;
	}

	return false;
}

template <typename T>
	requires (std::same_as<raw_type_t<T>, T> &&
			 (std::is_class_v<T> || std::is_enum_v<T>))
std::string_view LuaStateManager::GetRegisteredName() const
{
	if (IsNativeLuaType<T>())
	{
		return GetNativeLuaTypeName<T>();
	}

	if (auto idx = GetLuaUserTypeInfo().GetDataIndex<T>(); idx.IsValid())
	{
		return userTypeInfo_.GetUserTypeNames()[idx];
	}

	return {};
}