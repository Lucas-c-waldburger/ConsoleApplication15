#include "B2Chain.h"
#include "B2Shape.h"
#include <ranges>

std::vector<B2ChainSegmentShape> B2Chain::GetSegments() const
{
	const size_t segCount = GetSegmentCount();
	std::vector<b2ShapeId> segIds(segCount);

	const size_t actualCount = b2Chain_GetSegments(chainHandle_, segIds.data(), segCount);
	assert(actualCount == segCount);

	return segIds | std::views::transform([](auto id) {
		return B2ChainSegmentShape{ Handle<B2Shape>::Create(id) };
	}) | std::ranges::to<std::vector>();
}
