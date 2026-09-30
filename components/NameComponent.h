#pragma once
#include "ComponentConcepts.h"
#include <string>

struct Name
{
	std::string value;

	bool operator==(const Name&) const = default;
	auto operator<=>(const Name&) const = default;
	bool operator==(const char* ch) const { return value == ch; }
	auto operator<=>(const char* ch) const { return value <=> ch; }

	operator std::string_view() const noexcept { return value; }
};