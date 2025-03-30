#include "QuadTree.h"
#include "../../ecs/Ecs.h"

constexpr bool AABB::Contains(const AABB& other) const
{
	return (other.x >= x && other.x + other.w <= x + w) &&
		(other.y >= y && other.y + other.h <= y + h);
}

constexpr bool AABB::Intersects(const AABB& other) const
{
	return !(other.x + other.w < x || other.x > x + w ||
		other.y + other.h < y || other.y > y + h);
}

auto AABB::CreateFromEntity(const Entity& entity) -> AABB
{
	assert(entity.HasComponent<Collider>());

	const auto& collider = entity.GetComponent<Collider>();

	float width = collider.dimensions.w;
	float height = collider.dimensions.h;

	if (entity.HasComponent<Transform>())
	{
		const auto& tf = entity.GetComponent<Transform>();

		width *= tf.scale.x;
		height *= tf.scale.y;
	}

	return {
		collider.position.x - (width / 2.0f),
		collider.position.y - (height / 2.0f),
		width,
		height
	};
}

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

bool QuadTree::Node::Insert(Entity* entity)
{
	assert(entity);

	auto entityBounds = AABB::CreateFromEntity(*entity);

	if (!boundingBox.Intersects(entityBounds))
	{
		return false;
	}

	if (entities.size() < kMaxEntitiesPerNode || level == kMaxLevel)
	{
		// not subdividing
		entities.push_back(entity);
		return true;
	}

	// Otherwise, subdivide and insert entity into appropriate child node
	if (IsLeaf())
	{
		Subdivide();
	}

	for (auto& child : children)
	{
		assert(child);

		if (child->Insert(entity))
		{
			return true;
		}
	}

	return false;
}

// Get all entities in this node and its children that intersect a given bounding box
void QuadTree::Node::Query(const AABB& area, std::vector<Entity*>& result)
{
	if (!boundingBox.Intersects(area))
	{
		return;  // No need to check this node if the query area does not intersect it
	}

	// Add all entities that intersect with the query area
	for (Entity* entity : entities)
	{
		assert(entity);

		auto entityBounds = AABB::CreateFromEntity(*entity);

		if (area.Intersects(entityBounds))
		{
			result.push_back(entity);
		}
	}

	// Recursively check children
	if (!IsLeaf())
	{
		for (auto& child : children)
		{
			assert(child);

			child->Query(area, result);
		}
	}
}

// Query all entities within a given bounding box
std::vector<Entity*> QuadTree::Query(const AABB& area)
{
	std::vector<Entity*> result;
	root_.Query(area, result);

	return result;
}