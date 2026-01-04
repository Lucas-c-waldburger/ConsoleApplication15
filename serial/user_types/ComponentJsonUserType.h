#pragma once
#include "../SerializationConcepts.h"
#include "RenderableJsonUserTypes.h"
#include "PhysicsJsonUserTypes.h"
#include "../../components/ComponentIncludes.h"
#include "../../user/UserComponentIncludes.h"

#define DEF_COMPONENT_NAME(cmpType)										\
template <> struct ComponentName<cmpType> {								\
	static constexpr std::string_view value = #cmpType;					\
}																	    

#define DEF_COMPONENT_SERIALIZABLE(cmpType, ...)						\
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(cmpType, __VA_ARGS__)				\
DEF_COMPONENT_NAME(cmpType)

#define DEF_COMPONENT_SERIALIZABLE_EMPTY(cmpType)						\
DEF_SERIALIZABLE_EMPTY(cmpType)											\
DEF_COMPONENT_NAME(cmpType)


DEF_COMPONENT_SERIALIZABLE(Transform, position, rotation, scale);
DEF_COMPONENT_SERIALIZABLE(CameraTarget, offset, followSpeed);
DEF_COMPONENT_SERIALIZABLE(TextRenderableComponent, writer, formatting, profile);
DEF_COMPONENT_SERIALIZABLE(SpriteRenderableComponent, sprite, profile);

DEF_COMPONENT_NAME(RigidBody);
template <typename BasicJson> 
inline void to_json(BasicJson& j, const RigidBody& rb)
{
	//j["bodyLimits"] = rb.limits;
	//
	//auto& jBody = j["body"];
	//
	//const auto& body = rb.body.GetData();
	//if (!body.IsValid())
	//{
	//	jBody = nullptr;
	//	return;
	//}

	//jBody["handle"] = body.GetHandle().GetHash();

	//BodyParameters bodyParams{
	//	.bodyType = body.GetBodyType(),
	//	.position = body.GetPosition(),
	//	.gravityScale = body.GetGravityScale(),
	//	.fixedRotation = body.IsFixedRotation()
	//};
	//
	//jBody["bodyParameters"] = bodyParams;
}
template <typename BasicJson>
inline void from_json(const BasicJson& j, RigidBody& rb)
{
	//j.at("bodyLimits").get_to(rb.limits);
}

// COLLIDER
DEF_COMPONENT_NAME(Collider);
template <typename BasicJson>
inline void to_json(BasicJson& j, const Collider& c)
{
	/*auto& jShape = j["shape"];

	const auto& shape = c.shape.GetData();
	if (!shape.IsValid())
	{
		jShape = nullptr;
		return;
	}

	jShape["parentBody"] = shape.GetParentBodyHandle().GetHash();

	B2ShapeParameters shapeParams{
		.shapeType = shape.GetShapeType()
	};

	switch (shapeParams.shapeType)
	{
	case B2Shape::Type::Polygon:
		shapeParams.radius = shape.GetAs<B2PolygonShape>().GetRadius();
		break;
	case B2Shape::Type::Circle:
		shapeParams.radius = shape.GetAs<B2CircleShape>().GetRadius();
		break;
	default:
		break;
	}

	jShape["shapeParameters"] = std::move(shapeParams);

	jShape["colliderSettings"] = ColliderSettings{
		.density = shape.GetDensity(),
		.friction = shape.GetFriction(),
		.restitution = shape.GetRestitution(),
		.enableEvents = ColliderSettings::EnableEvents::FromEventsEnabled(
						shape.GetEventsEnabled()),
		.enableCollision = shape.IsCollisionEnabled(),
		.isSensor = shape.IsSensor()
	};*/
}
template <typename BasicJson>
inline void from_json(const BasicJson& j, Collider& c)
{}