#pragma once
#include "Ecs.h"
#include "../components/TransformComponent.h"
#include "../components/builder/RigidBodyComponentBuilder.h"
#include "../components/builder/ColliderComponentBuilder.h"
#include "../physics/B2World.h"
#include "../physics/B2Shape.h"

class EntityPhysics : HasWriteAccessImpl<EntityPhysics, B2Body, B2Shape>
{
public:
	struct Local
	{
		std::optional<SDL_FPoint> localPosition;
		std::optional<float> localRotation;
	};

	struct ColliderParams
	{
		ColliderSettings settings;
		B2CollisionFilter filter;
	};

	EntityPhysics() = default;
	EntityPhysics(const Entity& e, B2World* world) : entity_(e), world_(world) {}

	B2Body AddBody(B2Body::Type bodyType, SDL_FPoint pos);
	B2Body AddBody(BodyParameters params);

	Entity AddCollider(B2ShapeParameters sh, ColliderParams params);

	B2PolygonShape AddColliderBox(Dimensions<float> dims, Local local = {});
	B2PolygonShape AddColliderBox(Dimensions<float> dims, ColliderParams params,
								  Local local = {});

	B2PolygonShape AddColliderPoly(std::vector<SDL_FPoint> hull, Local local = {});
	B2PolygonShape AddColliderPoly(std::vector<SDL_FPoint> hull, ColliderParams params,
								   Local local = {});

	B2CircleShape AddColliderCircle(float radius, Local local = {});
	B2CircleShape AddColliderCircle(float radius, ColliderParams params,
									Local local = {});

	B2Body GetBody();
	const B2Body GetBody() const;
	std::vector<B2Shape> GetColliderShapes();
	std::vector<Entity> GetColliderEntities();

	bool HasBody() const;
	size_t GetColliderCount() const;

private:
	Entity GetOpenColliderEntity();
	B2Shape AddColliderImpl(B2ShapeParameters&& shapeParams, ColliderSettings&& settings,
							B2CollisionFilter&& filter);

	Entity entity_;
	B2World* world_ = nullptr;
};