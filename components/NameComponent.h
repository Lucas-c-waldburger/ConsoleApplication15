#pragma once
#include "BaseComponent.h"
#include <string>

struct Name : BaseComponent<Name>
{
	std::string value;

	friend auto operator<=>(const Name& lhs, const Name& rhs)
	{
		return lhs.value <=> rhs.value;
	}

	operator std::string_view() const noexcept
	{
		return value;
	}
};