#pragma once
#include <memory>


struct B2CollisionFilter
{
	uint64_t categories = 0x0000000000000001;
	uint64_t categoryMask = 0xFFFFFFFFFFFFFFFF;
	int groupIndex = 0;

	static int GetNextCollisionGroup(bool doCollide)
	{
		std::unique_ptr<int> nextDoCollideIndex;
		std::unique_ptr<int> nextNoCollideIndex;

		if (doCollide)
		{
			if (!nextDoCollideIndex)
			{
				nextDoCollideIndex = std::make_unique<int>(0);
			}

			return (*nextDoCollideIndex)++;
		}
		else
		{
			if (!nextNoCollideIndex)
			{
				nextNoCollideIndex = std::make_unique<int>(0);
			}

			return (*nextNoCollideIndex)++;
		}
	}
};