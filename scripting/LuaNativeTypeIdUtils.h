#pragma once
#include "../core/TypeUtils.h"
#include "../core/TypeInfo.h"

namespace detail {
struct InvalidLuaType;
struct LuaNativeBooleanType;
struct LuaNativeNumberType;
struct LuaNativeStringType;
} // detail

static constexpr uint32_t kInvalidLuaTypeId = TypeInfo<detail::InvalidLuaType>::hash32;
static constexpr uint32_t kNativeBooleanLuaTypeId = TypeInfo<detail::LuaNativeBooleanType>::hash32;
static constexpr uint32_t kNativeNumberLuaTypeId = TypeInfo<detail::LuaNativeNumberType>::hash32;
static constexpr uint32_t kNativeStringLuaTypeId = TypeInfo<detail::LuaNativeStringType>::hash32;

inline constexpr bool IsNativeLuaType(std::string_view name) noexcept
{
	return name == "boolean" || name == "number" || name == "string";
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
	else if constexpr (std::constructible_from<std::string, Raw> || std::same_as<Raw, char>)
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
	else
	{
		return kInvalidLuaTypeId;
	}
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
	else if constexpr (std::constructible_from<std::string, Raw> || std::same_as<Raw, char>)
	{
		return "string";
	}
	else
	{
		return "";
	}
}