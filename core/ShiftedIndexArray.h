#pragma once
#include <array>


template <typename T, size_t N, int shiftOffset>
class ShiftedIndexArray
{
public:
	ShiftedIndexArray() = default;

	T& operator[](size_t idx)
	{
		size_t realIdx = idx + shiftOffset;
		assert(realIdx < array_.size());

		return array_[realIdx];
	}

private:
	std::array<T, N> array_;
};