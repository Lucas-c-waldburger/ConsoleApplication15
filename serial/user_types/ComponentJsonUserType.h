#pragma once
#include "../SerializationConcepts.h"
#include "RenderableJsonUserTypes.h"
#include "AtlasJsonUserTypes.h"
#include "PhysicsJsonUserTypes.h"
#include "BitsetJsonUserTypes.h"
#include "AudioJsonUserTypes.h"
#include "../../components/ComponentIncludes.h"

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
DEF_COMPONENT_SERIALIZABLE(CameraTarget, offset, followSpeed, stopRadius);
DEF_COMPONENT_SERIALIZABLE(TextRenderableComponent, writer, formatting, profile);
DEF_COMPONENT_SERIALIZABLE(SpriteRenderableComponent, sprite, profile);
DEF_COMPONENT_SERIALIZABLE(SpriteAnimationComponent, spriteSeriesName, index);
DEF_COMPONENT_SERIALIZABLE(Tags, tags);
DEF_COMPONENT_SERIALIZABLE(GameControllerState, joystickID);
DEF_COMPONENT_SERIALIZABLE_EMPTY(MouseState);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EntityFlags, componentVisibilityFlags, eventProductionFlags)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Parent, entityId)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Children, childEntityIds)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Name, value)

//// TODO: Add Audio Stuff

DEF_COMPONENT_NAME(RigidBody);
template <typename BasicJson> 
inline void to_json(BasicJson& j, const RigidBody& rb)
{
	const auto& body = rb.body.GetData();
	auto& jId = j["bodyId"] = nullptr;
	auto& jParams = j["bodyParameters"] = nullptr;

	if (body.IsValid())
	{
		jId = body.GetHandle().GetHash();
		jParams = BodyParameters::FromB2Body(body);
	}

	j["bodyLimits"] = rb.limits;
}

template <typename BasicJson>
inline void from_json(const BasicJson& j, RigidBody& rb) {} 


// COLLIDER
DEF_COMPONENT_NAME(Collider);
template <typename BasicJson>
inline void to_json(BasicJson& j, const Collider& c)
{
	auto& parentIdJ = j["parentBodyId"] = nullptr;
	auto& settingsJ = j["colliderSettings"] = nullptr;
	auto& paramsJ	= j["shapeParameters"] = nullptr;

	const auto& shape = c.shape.GetData();
	if (!shape.IsValid())
	{
		return;
	}

	const auto& parentHandle = shape.GetParentBodyHandle();
	const auto parentBody = B2Body{ parentHandle };
	assert(parentBody.IsValid());

	parentIdJ = parentHandle.GetHash();
	settingsJ = ColliderSettings::FromB2Shape(shape);

	B2ShapeParameters shapeParams{
		.shapeType = shape.GetShapeType()
	}; 

	switch (shapeParams.shapeType)
	{
	case B2Shape::Type::Polygon:
	{
		const auto poly = shape.GetAs<B2PolygonShape>();
		shapeParams.hull = poly.GetVertices(B2Shape::CoordinateSpace::LocalSpace);
		break;
	}
	case B2Shape::Type::Circle:
	{
		const auto circle = shape.GetAs<B2CircleShape>();
		shapeParams.radius = circle.GetRadius();
		shapeParams.localPosition = circle.GetCenter(B2Shape::CoordinateSpace::LocalSpace);
		break;
	}
	default:
		break;
	}

	paramsJ = std::move(shapeParams);
}
template <typename BasicJson>
inline void from_json(const BasicJson& j, Collider& c)
{}


