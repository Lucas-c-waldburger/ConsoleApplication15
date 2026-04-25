#pragma once
#include <concepts>
#include <utility>

template <typename Enum>
concept SomeSizedEnum =
	std::is_enum_v<Enum> &&
	std::same_as<decltype(Enum::ENUM_SIZE_), Enum> && // has this value
	static_cast<std::underlying_type_t<Enum>>(Enum::ENUM_SIZE_) >= 0; // enum size non-negative

namespace detail {

template <SomeSizedEnum Enum>
struct enum_start { static constexpr size_t value = 0; };

} // detail

template <SomeSizedEnum Enum>
static constexpr size_t enum_start_v = detail::enum_start<Enum>::value;

template <SomeSizedEnum Enum>
static constexpr size_t enum_size_v = static_cast<size_t>(Enum::ENUM_SIZE_);

template <SomeSizedEnum Enum>
inline constexpr bool SizedEnumValueInRange(Enum e)
{
	const auto ul        = static_cast<std::underlying_type_t<Enum>>(e);
	constexpr auto start = static_cast<std::underlying_type_t<Enum>>(enum_start_v<Enum>);
	constexpr auto size  = static_cast<std::underlying_type_t<Enum>>(enum_size_v<Enum>);

	return ul >= start && ul < size;
}

template <SomeSizedEnum Enum>
using SizedEnumIndexSequence = std::index_sequence<enum_size_v<Enum>>;