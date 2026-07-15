#include "GuiEditPhysics.h"

#if IMGUI_ENABLED
#include "GuiEditCore.h"
#include "GuiEditPropertyTable.h"
#include "../../../physics/B2Body.h"

namespace ui {

bool GuiEdit(Force& f, const char* label)
{
	return GuiEditClass(f, label, [](auto& f) {
		bool b = GuiEdit(f.value, "value");
		b |= GuiEdit(f.worldPoint, "worldPoint");
		return b;
	});
}

bool GuiEdit(ForceRequests& fr, const char* label)
{
	return GuiEditClass(fr, label, [](auto& fr) {
		bool b = GuiEditContainer(fr.forces, "forces");
		b |= GuiEditContainer(fr.impulses, "impulses");
		return b;
	});
}

bool GuiEdit(BodyLimits& bl, const char* label)
{
	return GuiEditClass(bl, label, [](auto& bl) {
		bool b = GuiEdit(bl.linearVelocity, "linearVelocity");
		b |= GuiEdit(bl.angularVelocity, "angularVelocity");
		b |= GuiEdit(bl.maxImpulse, "maxImpulse");
		return b;
	});
}

// GUI EDIT PROPERTY //
bool GuiEditProperty(Force& f)
{
	bool b = Property("value", f.value);
	b |= Property("worldPoint", f.worldPoint);
	return b;
}

bool GuiEditProperty(ForceRequests& fr)
{
	bool b = PropertyGroup("forces", [&fr] { return Property("", fr.forces); });
	b |= PropertyGroup("impulses", [&fr] { return Property("", fr.impulses); });
	return b;
}

bool GuiEditProperty(BodyLimits& bl, ImGuiTreeNodeFlags flags)
{
	bool b = PropertyGroup("linearVelocity", [&bl] { return Property("", bl.linearVelocity); }, 
		{ .flags = flags });
	b |= PropertyGroup("angularVelocity", [&bl] { return Property("", bl.angularVelocity); }, 
		{ .flags = flags });
	b |= Property("maxImpulse", bl.maxImpulse);
	return b;
}

bool GuiEditProperty(B2Body::Type& bt)
{
	static constexpr const char* kNames[] = {
		"Static",
		"Kinematic",
		"Dynamic"
	};
	int cur = bt == B2Body::Type::Static ? 0 :
			  bt == B2Body::Type::Kinematic ? 1 :
			  bt == B2Body::Type::Dynamic ? 2 : 3;

	if (ImGui::Combo("##Value", &cur, kNames, IM_ARRAYSIZE(kNames)))
	{
		switch (cur)
		{
		case 0: bt = B2Body::Type::Static; break;
		case 1: bt = B2Body::Type::Kinematic; break;
		case 2: bt = B2Body::Type::Dynamic; break;
		}

		return true;
	}
	return false;
}

bool GuiEditProperty(B2Shape::Type& st)
{
	static constexpr const char* kNames[] = {
		"Circle",
		"Capsule",
		"Segment",
		"Polygon",
		"ChainSegment"
	};
	int cur = st == B2Shape::Type::Circle ? 0 :
			  st == B2Shape::Type::Capsule ? 1 :
			  st == B2Shape::Type::Segment ? 2 :
			  st == B2Shape::Type::Polygon ? 3 : 4;

	if (ImGui::Combo("##Value", &cur, kNames, IM_ARRAYSIZE(kNames)))
	{
		switch (cur)
		{
		case 0: st = B2Shape::Type::Circle; break;
		case 1: st = B2Shape::Type::Capsule; break;
		case 2: st = B2Shape::Type::Segment; break;
		case 3: st = B2Shape::Type::Polygon; break;
		case 4: st = B2Shape::Type::ChainSegment; break;
		}

		return true;
	}
	return false;
}

bool GuiEditProperty(RigidBody& rb)
{
	auto& body = WriteAccessor<B2Body>{}(rb.body);
	if (!body.IsValid())
	{
		return false;
	}

	const auto mass = body.GetMass();

	B2Body::Type type = body.GetBodyType();
	SDL_FPoint pos = body.GetPosition();
	float angle = body.GetAngle();
	SDL_FPoint linVel = body.GetLinearVelocity();
	float angVel = body.GetAngularVelocity();
	float linDamp = body.GetLinearDamping();
	float angDamp = body.GetAngularDamping();
	float gravScale = body.GetGravityScale();
	bool fixedRot = body.IsFixedRotation();
	bool awake = body.IsAwake();

	bool b = false;
	
	Property("mass", mass);

	if (Property("bodyType", type))
	{
		body.SetBodyType(type);
		b |= true;
	}
	if (Property("position", pos))
	{
		body.SetPosition(pos);
		b |= true;
	}
	if (Property("angle", angle))
	{
		body.SetAngle(angle);
		b |= true;
	}
	if (Property("linearVelocity", linVel))
	{
		body.SetLinearVelocity(linVel);
		b |= true;
	}
	if (Property("angularVelocity", angVel))
	{
		body.SetAngularVelocity(angVel);
		b |= true;
	}
	if (Property("linearDamping", linDamp))
	{
		body.SetLinearDamping(linDamp);
		b |= true;
	}
	if (Property("angularDamping", angDamp))
	{
		body.SetAngularDamping(angDamp);
		b |= true;
	}
	if (Property("gravityScale", gravScale))
	{
		body.SetGravityScale(gravScale);
		b |= true;
	}
	if (Property("fixedRotation", fixedRot))
	{
		body.SetFixedRotation(fixedRot);
		b |= true;
	}
	if (Property("awake", awake))
	{
		body.SetAwake(awake);
		b |= true;
	}

	b |= PropertyGroup("limits", [&rb] { return Property("", rb.limits); });

	return b;
}

bool GuiEditProperty(Collider& col)
{
	auto& shape = WriteAccessor<B2Shape>{}(col.shape);
	if (!shape.IsValid())
	{
		return false;
	}

	const B2Shape::Type type = shape.GetShapeType();
	const std::string_view typeStr = ToString(type);
	const bool isCollisionEnabled = shape.IsCollisionEnabled();
	const bool isSensor = shape.IsSensor();
	const SDL_FRect bbox = shape.GetBoundingBox();

	auto density = shape.GetDensity();
	auto friction = shape.GetFriction();
	auto restitution = shape.GetRestitution();

	Property("type", typeStr);
	Property("collisionEnabled", isCollisionEnabled);
	Property("isSensor", isSensor);
	Property("boundingBox", bbox);

	if (type == B2Shape::Type::Polygon)
	{
		const float radius = shape.GetAs<B2PolygonShape>().GetRadius();
		Property("radius", radius);
	}
	else if (type == B2Shape::Type::Circle)
	{
		auto circle = shape.GetAs<B2CircleShape>();
		const float radius = circle.GetRadius();
		const SDL_FPoint center = circle.GetCenter();

		Property("radius", radius);
		Property("center", center);
	}

	bool b = false;

	if (Property("density", density))
	{
		shape.SetDensity(density);
		b |= true;
	}
	if (Property("friction", friction))
	{
		shape.SetFriction(friction);
		b |= true;
	}
	if (Property("restitution", restitution))
	{
		shape.SetRestitution(restitution);
		b |= true;
	}

	return b;
}


} // ui

#endif