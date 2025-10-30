#pragma once
#include <string_view>
#include <string>

namespace detail {
template<typename ... Bases>
struct OverloadSet : Bases ...
{
	using is_transparent = void;
	using Bases::operator() ...;
};

struct CharPtrHash
{
	auto operator()(const char* ptr) const noexcept
	{
		return std::hash<std::string_view>{}(ptr);
	}
};
} // detail

using TransparentStringHash = detail::OverloadSet<
	std::hash<std::string>,
	std::hash<std::string_view>,
	detail::CharPtrHash
>;