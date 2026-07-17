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
PropertyEditState GuiEditProperty(Force& f)
{
	auto state = Property("value", f.value);
	state |= Property("worldPoint", f.worldPoint);
	return state;
}

PropertyEditState GuiEditProperty(ForceRequests& fr)
{
	auto state = PropertyGroup("forces", [&fr] { return Property("", fr.forces); });
	state |= PropertyGroup("impulses", [&fr] { return Property("", fr.impulses); });
	return state;
}

PropertyEditState GuiEditProperty(BodyLimits& bl, ImGuiTreeNodeFlags flags)
{
	auto state = PropertyGroup("linearVelocity", [&bl] { return Property("", bl.linearVelocity); }, 
		{ .flags = flags });
	state |= PropertyGroup("angularVelocity", [&bl] { return Property("", bl.angularVelocity); },
		{ .flags = flags });
	state |= Property("maxImpulse", bl.maxImpulse);
	return state;
}

PropertyEditState GuiEditProperty(B2Body::Type& bt)
{
	static constexpr const char* kNames[] = {
		"Static",
		"Kinematic",
		"Dynamic"
	};
	int cur = bt == B2Body::Type::Static ? 0 :
			  bt == B2Body::Type::Kinematic ? 1 :
			  bt == B2Body::Type::Dynamic ? 2 : 3;

	bool changed = ImGui::Combo("##Value", &cur, kNames, IM_ARRAYSIZE(kNames));

	PropertyEditState state = PropertyEditState::None;

	if (ImGui::IsItemActivated())
	{
		state = PropertyEditState::Started;
	}

	if (changed)
	{
		switch (cur)
		{
		case 0: bt = B2Body::Type::Static; break;
		case 1: bt = B2Body::Type::Kinematic; break;
		case 2: bt = B2Body::Type::Dynamic; break;
		}

		state = PropertyEditState::Finished;
	}
	else if (state != PropertyEditState::Started && ImGui::IsItemActive())
	{
		state = PropertyEditState::Active;
	}
	
	return state;
}

PropertyEditState GuiEditProperty(B2Shape::Type& st)
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

	const bool changed = ImGui::Combo("##Value", &cur, kNames, IM_ARRAYSIZE(kNames));

	PropertyEditState state = PropertyEditState::None;

	if (ImGui::IsItemActivated())
	{
		state = PropertyEditState::Started;
	}

	if (changed)
	{
		switch (cur)
		{
		case 0: st = B2Shape::Type::Circle; break;
		case 1: st = B2Shape::Type::Capsule; break;
		case 2: st = B2Shape::Type::Segment; break;
		case 3: st = B2Shape::Type::Polygon; break;
		case 4: st = B2Shape::Type::ChainSegment; break;
		}

		state = PropertyEditState::Finished;
	}
	else if (state != PropertyEditState::Started && ImGui::IsItemActive())
	{
		state = PropertyEditState::Active;
	}

	return state;
}

PropertyEditState GuiEditProperty(RigidBody& rb)
{
	auto& body = WriteAccessor<B2Body>{}(rb.body);
	if (!body.IsValid())
	{
		return PropertyEditState::None;
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

	auto state = PropertyEditState::None;
	
	Property("mass", mass);

	auto st = Property("bodyType", type);
	if (st != PropertyEditState::None)
	{
		body.SetBodyType(type);
	}
	state |= st;

	st = Property("position", pos);
	if (st != PropertyEditState::None)
	{
		body.SetPosition(pos);
	}
	state |= st;

	st = Property("angle", angle);
	if (st != PropertyEditState::None)
	{
		body.SetAngle(angle);
	}
	state |= st;

	st = Property("linearVelocity", linVel);
	if (st != PropertyEditState::None)
	{
		body.SetLinearVelocity(linVel);
	}
	state |= st;

	st = Property("angularVelocity", angVel);
	if (st != PropertyEditState::None)
	{
		body.SetAngularVelocity(angVel);
	}
	state |= st;

	st = Property("linearDamping", linDamp);
	if (st != PropertyEditState::None)
	{
		body.SetLinearDamping(linDamp);
	}
	state |= st;

	st = Property("angularDamping", angDamp);
	if (st != PropertyEditState::None)
	{
		body.SetAngularDamping(angDamp);
	}
	state |= st;

	st = Property("gravityScale", gravScale);
	if (st != PropertyEditState::None)
	{
		body.SetGravityScale(gravScale);
	}
	state |= st;

	st = Property("fixedRotation", fixedRot);
	if (st != PropertyEditState::None)
	{
		body.SetFixedRotation(fixedRot);
	}
	state |= st;

	st = Property("awake", awake);
	if (st != PropertyEditState::None)
	{
		body.SetAwake(awake);
	}
	state |= st;

	state |= PropertyGroup("limits", [&rb] { return Property("", rb.limits); });

	return state;
}

PropertyEditState GuiEditProperty(Collider& col)
{
	auto& shape = WriteAccessor<B2Shape>{}(col.shape);
	if (!shape.IsValid())
	{
		return PropertyEditState::None;
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

	PropertyEditState state = PropertyEditState::None;

	auto st = Property("density", density);
	if (st != PropertyEditState::None)
	{
		shape.SetDensity(density);
	}
	state |= st;

	st = Property("friction", friction);
	if (st != PropertyEditState::None)
	{
		shape.SetFriction(friction);
	}
	state |= st;

	st = Property("restitution", restitution);
	if (st != PropertyEditState::None)
	{
		shape.SetRestitution(restitution);
	}
	state |= st;

	return state |= st;
}


} // ui

#endif