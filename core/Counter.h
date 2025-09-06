#pragma once
#include <cstdint>

class Counter
{
public:
	float GetDelta() const
	{
		return static_cast<float>(delta_);
	}

	void Update();

private:
	uint64_t last_ = 0;
	double delta_ = 0.0;
};