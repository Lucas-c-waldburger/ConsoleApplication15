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

	EntityPhysics() = default;
	EntityPhysics(const Entity& e, B2World* world) : entity_(e), world_(world) {}

	B2Body AddBody(B2Body::Type bodyType, SDL_FPoint pos);
	B2Body AddBody(BodyParameters params);

	B2PolygonShape AddColliderBox(Dimensions<float> dims, Local local = {});
	B2PolygonShape AddColliderBox(Dimensions<float> dims, ColliderSettings settings,
								  Local local = {});

	B2PolygonShape AddColliderPoly(std::vector<SDL_FPoint> hull, Local local = {});
	B2PolygonShape AddColliderPoly(std::vector<SDL_FPoint> hull, ColliderSettings settings, 
								   Local local = {});

	B2CircleShape AddColliderCircle(float radius, Local local = {});
	B2CircleShape AddColliderCircle(float radius, ColliderSettings settings, 
									Local local = {});

	B2Body GetBody();
	const B2Body GetBody() const;
	std::vector<B2Shape> GetColliderShapes();
	std::vector<Entity> GetColliderEntities();

	bool HasBody() const;
	size_t GetColliderCount() const;

private:
	Entity GetOpenColliderEntity();
	B2Shape AddColliderImpl(B2ShapeParameters&& shapeParams, ColliderSettings&& settings);

	Entity entity_;
	B2World* world_ = nullptr;
};