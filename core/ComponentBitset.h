#pragma once
#include "../components/ComponentConcepts.h"

class ComponentBitset
{
public:
	void Reset() { bitset_ = 0; }

	void Set() { bitset_ = 0xFFFFFFFFFFFFFFFF; }

	template <SomeComponent...Ts>
	void Set(bool tf)
	{
		((SetInternal<Ts>(tf)), ...);
	}

	template <SomeComponent T>
	constexpr bool Test() const { return (bitset_ & T::componentBit) != 0; }

private:
	template <SomeComponent T>
	void SetInternal(bool tf)
	{
		if (tf)
		{
			bitset_ |= T::componentBit;
		}
		else
		{
			bitset_ &= ~(T::componentBit);
		}
	}

	uint64_t bitset_ = 0;
};