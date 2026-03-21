#include "EntityPhysics.h"
#include <ranges>

namespace {

bool IsBodyValid(const RigidBody& rigid)
{
	return rigid.body.GetData().IsValid();
}
bool IsShapeValid(const Collider& collider)
{
	return collider.shape.GetData().IsValid();
}

} // unnamed

B2Body EntityPhysics::AddBody(BodyParameters params)
{
	if (!entity_.IsValid())
	{
		return {};
	}
	if (!world_)
	{
		return {};
	}

	entity_.AddComponent<Transform>().position = params.position;

	auto& rigid = entity_.AddComponent<RigidBody>();
	if (rigid.body.GetData().IsValid())
	{
		GetWriteAccess(rigid.body).Destroy();
	}

	rigid = ComponentBuilder<RigidBody>{}
		.WithBodyParameters(std::move(params))
		.Build(*world_);

	auto& body = GetWriteAccess(rigid.body);
	assert(body.IsValid());

	return body;
}

B2Body EntityPhysics::AddBody(B2Body::Type bodyType, SDL_FPoint pos)
{
	return AddBody({ .bodyType = bodyType, .position = pos });
}

B2Shape EntityPhysics::AddColliderImpl(B2ShapeParameters&& shapeParams, 
									  ColliderSettings&& settings)
{
	if (!HasBody())
	{
		return {};
	}

	auto& body = GetWriteAccess(entity_.GetComponent<RigidBody>().body);

	auto colliderEnt = GetOpenColliderEntity();
	if (!colliderEnt.IsValid())
	{
		return {};
	}

	auto& collider = colliderEnt.AddComponent(ComponentBuilder<Collider>{}
		.WithShapeParameters(std::move(shapeParams))
		.WithColliderSettings(std::move(settings))
		.Build(body)
	);

	auto& shape = GetWriteAccess(collider.shape);
	assert(shape.IsValid());

	return shape;
}

B2PolygonShape EntityPhysics::AddColliderBox(Dimensions<float> dims, ColliderSettings settings,
											 Local local)
{
	return AddColliderImpl({
		.shapeType = B2Shape::Type::Polygon,
		.dimensions = dims,
		.localPosition = std::move(local.localPosition),
		.localRotation = std::move(local.localRotation)
	}, std::move(settings)).GetAs<B2PolygonShape>();
}

B2PolygonShape EntityPhysics::AddColliderBox(Dimensions<float> dims, Local local)
{
	return AddColliderBox(dims, {}, local);
}

B2PolygonShape EntityPhysics::AddColliderPoly(std::vector<SDL_FPoint> hull, 
											  ColliderSettings settings, Local local)
{
	return AddColliderImpl({
		.shapeType = B2Shape::Type::Polygon,
		.hull = std::move(hull),
		.localPosition = std::move(local.localPosition),
		.localRotation = std::move(local.localRotation)
	}, std::move(settings)).GetAs<B2PolygonShape>();
}

B2PolygonShape EntityPhysics::AddColliderPoly(std::vector<SDL_FPoint> hull, Local local)
{
	return AddColliderPoly(std::move(hull), {}, local);
}

B2CircleShape EntityPhysics::AddColliderCircle(float radius, ColliderSettings settings, 
											   Local local)
{
	return AddColliderImpl({
		.shapeType = B2Shape::Type::Circle,
		.localPosition = std::move(local.localPosition),
		.localRotation = std::move(local.localRotation),
		.radius = radius
	}, std::move(settings)).GetAs<B2CircleShape>();
}

B2CircleShape EntityPhysics::AddColliderCircle(float radius, Local local)
{
	return AddColliderCircle(radius, {}, local);
}

B2Body EntityPhysics::GetBody()
{
	return (entity_.HasComponent<RigidBody>())
		? GetWriteAccess(entity_.GetComponent<RigidBody>().body)
		: B2Body{};
}

const B2Body EntityPhysics::GetBody() const
{
	return (entity_.HasComponent<RigidBody>())
		? entity_.GetComponent<RigidBody>().body.GetData()
		: B2Body{};
}

std::vector<B2Shape> EntityPhysics::GetColliderShapes()
{
	//auto body = GetBody();
	//if (!body.IsValid())
	//{
	//	return {};
	//}

	//return body.GetShapes();

	return GetColliderEntities() | std::views::transform([this](Entity& e) {
		return GetWriteAccess(e.GetComponent<Collider>().shape);
	}) | std::ranges::to<std::vector>();
}

std::vector<Entity> EntityPhysics::GetColliderEntities()
{
	const size_t colliderCount = GetColliderCount();
	if (colliderCount == 0)
	{
		return {};
	}

	std::vector<Entity> entities;
	entities.reserve(colliderCount);
	entities.emplace_back(entity_);
	
	if (colliderCount > 1)
	{
		auto rels = entity_.GetRelations();
		assert(rels.IsParent());

		auto children = rels.GetAllChildrenWith<Collider>(&IsShapeValid);

		entities.insert(entities.end(), std::make_move_iterator(children.begin()), 
										std::make_move_iterator(children.end()));
	}

	return entities;
}

size_t EntityPhysics::GetColliderCount() const
{
	const auto body = GetBody();

	return body.IsValid() ? body.GetShapeCount() : 0;
}

bool EntityPhysics::HasBody() const
{
	return entity_.HasComponent<RigidBody>(&IsBodyValid);
}

Entity EntityPhysics::GetOpenColliderEntity()
{
	if (!entity_.IsValid())
	{
		return {};
	}

	if (!entity_.HasComponent<Collider>(&IsShapeValid))
	{
		return entity_;
	}

	auto rels = entity_.GetRelations();
	if (rels.IsChild())
	{
		return {};
	}

	auto ch = rels.AddChild();
	ch.AddComponent<Transform>();

	return ch;
}