#include "QuadTree.h"
#include "../../ecs/Ecs.h"

void QuadTree::Node::Subdivide()
{
	if (level >= kMaxLevel)
	{
		return;
	}

	float halfWidth = boundingBox.w / 2;
	float halfHeight = boundingBox.h / 2;

	AABB topLeft{ boundingBox.x, boundingBox.y, halfWidth, halfHeight };
	AABB topRight{ boundingBox.x + halfWidth, boundingBox.y, halfWidth, halfHeight };
	AABB botLeft{ boundingBox.x, boundingBox.y + halfHeight, halfWidth, halfHeight };
	AABB botRight{ boundingBox.x + halfWidth, boundingBox.y + halfHeight, halfWidth, halfHeight };

	children[0] = std::make_unique<Node>(topLeft, level + 1);
	children[1] = std::make_unique<Node>(topRight, level + 1);
	children[2] = std::make_unique<Node>(botLeft, level + 1);
	children[3] = std::make_unique<Node>(botRight, level + 1);
}

bool QuadTree::Node::Insert(const Entity_t entityId, const AABB& bounds)
{
	if (!boundingBox.Intersects(bounds))
	{
		return false;
	}

	if (entityIds.Size() < kMaxEntitiesPerNode || level == kMaxLevel)
	{
		// not subdividing
		return entityIds.Insert(entityId);
	}

	// Otherwise, subdivide and insert entity into appropriate child node
	if (IsLeaf())
	{
		Subdivide();
	}

	for (auto& child : children)
	{
		assert(child);

		if (child->Insert(entityId, bounds))
		{
			return true;
		}
	}

	return false;
}

// Get all entities in this node and its children that intersect a given bounding box
void QuadTree::Node::Query(const AABB& area, std::vector<EntityColliderBounds>& result, uint8_t flagsFilter)
{
	if (!boundingBox.Intersects(area))
	{
		return;  // No need to check this node if the query area does not intersect it
	}

	size_t i = 0;
	while (i < entityIds.Size())
	{
		auto entity = ECS::GetEntityByID(entityIds[i]);

		auto entityBounds = EntityColliderBounds::CreateFromEntity(entity, flagsFilter);
		if (!entityBounds.has_value())
		{
			assert(entityIds.Erase(entityIds[i]));
		}
		else
		{
			if (area.Intersects(entityBounds->boundingBox))
			{
				result.push_back(*entityBounds);
			}

			++i;
		}
	}

	if (!IsLeaf())
	{
		for (auto& child : children)
		{
			assert(child);

			child->Query(area, result, flagsFilter);
		}
	}
}

// Query all entities within a given bounding box
std::vector<EntityColliderBounds> QuadTree::GetIntersecting(const AABB& area, uint8_t flagsFilter)
{
	std::vector<EntityColliderBounds> result;
	root_.Query(area, result, flagsFilter);

	return result;
}
