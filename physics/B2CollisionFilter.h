#pragma once
#include <memory>
#include "box2d/collision.h"


struct NewB2CollisionFilter
{
	uint64_t category    = 0xFFFFFFFFFFFFFFFFULL; 
	uint64_t collideWith = 0xFFFFFFFFFFFFFFFFULL;
	int overrideGroup    = 0;
};
//
//
//
//struct OverrideCollisionGroup
//{
//	static int NewGroup(bool doCollide) noexcept
//	{
//		auto& idx = (doCollide) ? nextDoCollideIndex : nextNoCollideIndex;
//		if (!idx.has_value())
//		{
//			idx.emplace(0);
//		}
//
//		return ++(*idx);
//	}
//
//	static inline std::optional<int> nextDoCollideIndex{};
//	static inline std::optional<int> nextNoCollideIndex{};
//};
//
//template <typename E> requires std::is_enum_v<E>
//struct CollisionKey
//{
//	template <E enumVal, bool doCollide>
//	struct Value
//	{
//		int val = 0;
//		operator int()() { return val; }
//	};
//
//	template <E enumVal, bool doCollide>
//	static int value() noexcept
//	{
//		static int val = OverrideCollisionGroup::NewGroup(doCollide);
//		return val;
//	}
//
//	friend bool operator==(const CollisionKey& lhs, const CollisionKey& rhs) noexcept
//	{
//
//	}
//};


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