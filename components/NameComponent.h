#pragma once
#include "BaseComponent.h"
#include <string>

struct Name : BaseComponent<Name>
{
	std::string value;


	friend bool operator==(const Name& lhs, const Name& rhs)
	{
		return lhs.value == rhs.value;
	}
	friend auto operator<=>(const Name& lhs, const Name& rhs)
	{
		return lhs.value <=> rhs.value;
	}
	friend bool operator==(const Name& lhs, const char* ch)
	{
		return lhs.value == ch;
	}
	friend auto operator<=>(const Name& lhs, const char* ch)
	{
		return lhs.value <=> ch;
	}

	operator std::string_view() const noexcept
	{
		return value;
	}
};