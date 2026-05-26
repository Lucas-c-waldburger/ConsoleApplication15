#pragma once
#include "../../ecs/Ecs.h"
#include "CoreJsonUserTypes.h"
#include "SDLJsonUserTypes.h"
#include "../../components/builder/RigidBodyComponentBuilder.h"
#include "../../components/builder/ColliderComponentBuilder.h"



NLOHMANN_JSON_SERIALIZE_ENUM(
	B2Body::Type,
	{
		{ B2Body::Type::Static,    "Static" },
		{ B2Body::Type::Kinematic, "Kinematic" },
		{ B2Body::Type::Dynamic,   "Dynamic" }
	}
)

NLOHMANN_JSON_SERIALIZE_ENUM(
	B2Shape::Type,
	{
		{ B2Shape::Type::Invalid,      "Invalid" },
		{ B2Shape::Type::Circle,	   "Circle" },
		{ B2Shape::Type::Capsule,	   "Capsule" },
		{ B2Shape::Type::Polygon,	   "Polygon" },
		{ B2Shape::Type::Segment,	   "Segment" },
		{ B2Shape::Type::ChainSegment, "ChainSegment" }
	}
) 

// BODY
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BodyParameters, bodyType, position, gravityScale,
								   fixedRotation)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BodyLimits, linearVelocity, angularVelocity)


template <typename BasicJson>
inline void from_json(const BasicJson& j, ComponentBuilder<RigidBody>& builder)
{
	if (j.contains("bodyParameters"))
	{
		builder.WithBodyParameters(j.at("bodyParameters").get<BodyParameters>());
	}
	if (j.contains("bodyLimits"))
	{
		builder.WithBodyLimits(j.at("bodyLimits").get<BodyLimits>());
	}
}

// COLLIDER
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ColliderSettings::EnableEvents, contact, sensor, hit)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ColliderSettings, density, friction, restitution, 
									enableEvents, enableCollision, isSensor)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(B2ShapeParameters, shapeType, dimensions, hull,
									localPosition, localRotation, radius)

template <typename BasicJson>
inline void to_json(BasicJson& j, const ReadOnly<B2Body>& roBody)
{
	if (roBody.GetData().IsValid())
	{
		j["bodyType"] = roBody.GetData().GetBodyType();
	}
	else
	{
		j["bodyType"] = nullptr;
	}
}
template <typename BasicJson>
inline void from_json(const BasicJson& j, ReadOnly<B2Body>& roBody) {}

template <typename BasicJson>
inline void to_json(BasicJson& j, const ReadOnly<B2Shape>& roShape)
{
	if (roShape.GetData().IsValid())
	{
		j["shapeType"] = roShape.GetData().GetShapeType();
	}
	else
	{
		j["shapeType"] = B2Shape::Type::Invalid;
	}
}
template <typename BasicJson>
inline void from_json(const BasicJson& j, ReadOnly<B2Shape>& roShape) {}

