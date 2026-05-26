#pragma once
#include <charconv>
#include <string_view>
#include "Result.h"


inline std::string_view TrimWhitespace(std::string_view sv)
{
	const auto first = sv.find_first_not_of(' ');
	if (first == std::string_view::npos)
	{
		return {};
	}

	const auto last = sv.find_last_not_of(' ');

	return sv.substr(first, (last - first + 1));
}

inline Result<int> StringViewToInt(std::string_view sv)
{
	int val = 0;
	auto [_, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), val);

	if (ec != std::errc())
	{
		return MAKE_ERROR_FMT("Int conversion failed for string_view '{}'", sv);
	}

	return val;
}