#pragma once
#include <typeinfo>
#include "Result.h"
#include "commonObjects.h"


struct TypeDomain
{
	TypeDomain() {}

	friend bool operator==(const TypeDomain& dom, const std::type_info& ti)
	{
		return dom.typeInfo_ && *dom.typeInfo_ == ti;
	}

	template <typename T>
	static TypeDomain Create()
	{
		return TypeDomain{ typeid(T) };
	}

private:
	explicit TypeDomain(const std::type_info& ti) : typeInfo_(&ti) {}

	const std::type_info* typeInfo_ = nullptr;
};

static_assert(std::is_swappable_v<TypeDomain>);