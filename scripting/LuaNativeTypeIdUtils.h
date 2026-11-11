#pragma once
#include "../core/TypeUtils.h"
#include "../core/TypeInfo.h"
#include <sol/sol.hpp>

namespace detail {
struct InvalidLuaType;
struct LuaNativeBooleanType;
struct LuaNativeNumberType;
struct LuaNativeStringType;
struct LuaNativeTableType;
struct LuaNativeFunctionType;

template <typename> struct is_sol_basic_function : std::false_type {};
template <typename T, bool b> 
struct is_sol_basic_function<sol::basic_function<T, b>> : std::true_type {};

template <typename> struct is_sol_basic_protected_function : std::false_type {};
template <typename T, bool b, typename H>
struct is_sol_basic_protected_function<sol::basic_protected_function<T, b, H>>
	: std::true_type {};

template <typename T>
inline constexpr bool is_sol_function_v = 
	(is_sol_basic_function<T>::value || is_sol_basic_protected_function<T>::value);

} // detail

static constexpr uint32_t kInvalidLuaTypeId = TypeInfo<detail::InvalidLuaType>::hash32;
static constexpr uint32_t kNativeBooleanLuaTypeId = TypeInfo<detail::LuaNativeBooleanType>::hash32;
static constexpr uint32_t kNativeNumberLuaTypeId = TypeInfo<detail::LuaNativeNumberType>::hash32;
static constexpr uint32_t kNativeStringLuaTypeId = TypeInfo<detail::LuaNativeStringType>::hash32;
static constexpr uint32_t kNativeTableLuaTypeId = TypeInfo<detail::LuaNativeTableType>::hash32;
static constexpr uint32_t kNativeFunctionLuaTypeId = TypeInfo<detail::LuaNativeFunctionType>::hash32;

inline constexpr bool IsNativeLuaType(std::string_view name) noexcept
{
	return name == "boolean" || name == "number" || name == "string" ||
		   name == "table" || name == "function";
}

template <typename T>
inline constexpr bool IsNativeLuaType() noexcept
{
	using Raw = raw_type_t<T>;

	if constexpr (std::same_as<Raw, bool>)
	{
		return true;
	}
	else if constexpr (std::is_arithmetic_v<Raw> && !std::same_as<Raw, char>)
	{
		return true;
	}
	else if constexpr (std::constructible_from<std::string, Raw> || 
					   std::same_as<Raw, char>)
	{
		return true;
	}
	else if constexpr (std::same_as<Raw, sol::table>)
	{
		return true;
	}
	else if constexpr (detail::is_sol_function_v<Raw>)
	{
		return true;
	}
	else
	{
		return false;
	}
}

template <typename T>
inline constexpr uint32_t GetNativeLuaTypeId() noexcept
{
	using Raw = raw_type_t<T>;

	if constexpr (std::same_as<Raw, bool>)
	{
		return kNativeBooleanLuaTypeId;
	}
	else if constexpr (std::is_arithmetic_v<Raw> && !std::same_as<Raw, char>)
	{
		return kNativeNumberLuaTypeId;
	}
	else if constexpr (std::constructible_from<std::string, Raw> ||
					  std::same_as<Raw, char>)
	{
		return kNativeStringLuaTypeId;
	}
	else if constexpr (std::same_as<Raw, sol::table>)
	{
		return kNativeTableLuaTypeId;
	}
	else if constexpr (detail::is_sol_function_v<Raw>)
	{
		return kNativeFunctionLuaTypeId;
	}
	else
	{
		return kInvalidLuaTypeId;
	}
}

inline uint32_t GetNativeLuaTypeId(std::string_view name)
{
	if (name == "boolean")
	{
		return kNativeBooleanLuaTypeId;
	}
	if (name == "number")
	{
		return kNativeNumberLuaTypeId;
	}
	if (name == "string")
	{
		return kNativeStringLuaTypeId;
	}
	if (name == "table")
	{
		return kNativeTableLuaTypeId;
	}
	if (name == "function")
	{
		return kNativeFunctionLuaTypeId;
	}

	return kInvalidLuaTypeId;
}

template <typename T>
inline constexpr std::string_view GetNativeLuaTypeName() noexcept
{
	using Raw = raw_type_t<T>;

	if constexpr (std::same_as<Raw, bool>)
	{
		return "boolean";
	}
	else if constexpr (std::is_arithmetic_v<Raw> && !std::same_as<Raw, char>)
	{
		return "number";
	}
	else if constexpr (std::constructible_from<std::string, Raw> || 
					   std::same_as<Raw, char>)
	{
		return "string";
	}
	else if constexpr (std::same_as<Raw, sol::table>)
	{
		return "table";
	}
	else if constexpr (detail::is_sol_function_v<Raw>)
	{
		return "function";
	}
	else
	{
		return "";
	}
}