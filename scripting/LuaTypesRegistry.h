#pragma once
#include <sol/sol.hpp>
#include <lua.hpp>
#include <filesystem>
#include "../components/ComponentRegistry.h"
#include "../core/commonObjects.h"
#include "../core/Handle.h"
#include "ScriptInfo.h"
#include <SDL.h>
#include "../physics/B2Body.h"

template <typename>
static void RegisterLuaUserType(sol::state& lua);

// SDL TYPES
template <> static void RegisterLuaUserType<SDL_FPoint>(sol::state& lua)
{
	try {
		lua.new_usertype<SDL_FPoint>("SDL_FPoint", "x", &SDL_FPoint::x, "y", &SDL_FPoint::y);
	} catch (sol::error& e) { std::cout << e.what() << '\n'; }
}
template <> static void RegisterLuaUserType<SDL_Point>(sol::state& lua)
{
	lua.new_usertype<SDL_Point>("SDL_Point", "x", &SDL_Point::x, "y", &SDL_Point::y);
}
template <> static void RegisterLuaUserType<SDL_Rect>(sol::state& lua)
{
	lua.new_usertype<SDL_Rect>("SDL_Rect", "x", &SDL_Rect::x, "y", &SDL_Rect::y,
										   "w", &SDL_Rect::w, "h", &SDL_Rect::h);
}
template <> static void RegisterLuaUserType<SDL_FRect>(sol::state& lua)
{
	lua.new_usertype<SDL_FRect>("SDL_FRect", "x", &SDL_FRect::x, "y", &SDL_FRect::y,
					 						 "w", &SDL_FRect::w, "h", &SDL_FRect::h);
}

// my core types
template <> static void RegisterLuaUserType<Dimensions<float>>(sol::state& lua)
{
	try {
		lua.new_usertype<Dimensions<float>>("Dimensions<float>", 
			"w", &Dimensions<float>::w, "h", &Dimensions<float>::h);
	} catch (sol::error& e) { std::cout << e.what() << '\n'; }
}


//// HANDLES
//class GlyphAtlas;
//template <> static void RegisterLuaUserType<Handle<GlyphAtlas>>(sol::state& lua)
//{
//	lua.new_usertype<Handle<GlyphAtlas>>("Handle<GlyphAtlas>",
//		sol::meta_function::equal_to, &Handle<GlyphAtlas>::operator==);
//
//	lua["Handle<GlyphAtlas>"]["__ne"] = [](const Handle<GlyphAtlas>& lhs, const Handle<GlyphAtlas>& rhs) {
//		return lhs != rhs;
//	};
//}
//class SpriteSeriesAtlas;
//template <> static void RegisterLuaUserType<Handle<SpriteSeriesAtlas>>(sol::state& lua)
//{
//	lua.new_usertype<Handle<SpriteSeriesAtlas>>("Handle<SpriteSeriesAtlas>",
//		sol::meta_function::equal_to, &Handle<SpriteSeriesAtlas>::operator==);
//
//	lua["Handle<SpriteSeriesAtlas>"]["__ne"] = 
//		[](const Handle<SpriteSeriesAtlas>& lhs, const Handle<SpriteSeriesAtlas>& rhs) {
//			return lhs != rhs;
//	};
//}
//template <> static void RegisterLuaUserType<Force>(sol::state& lua)
//{
//	lua.new_usertype<Force>("Force", "vector", &Force::vector, "duration", &Force::duration);
//}
//template <> static void RegisterLuaUserType<AccumulatedForces>(sol::state& lua)
//{
//	lua.new_usertype<AccumulatedForces>("AccumulatedForces", "normed",
//		&AccumulatedForces::normed, "max", &AccumulatedForces::max);
//}
//template <> static void RegisterLuaUserType<Collider::Material>(sol::state& lua)
//{
//	lua.new_usertype<Collider::Material>("Collider::Material",
//		"restitution", &Collider::Material::restitution, "friction", &Collider::Material::friction);
//}

template <> static void RegisterLuaUserType<SDL_GameControllerButton>(sol::state& lua)
{
	lua.new_enum("GameControllerButton",
		"Invalid", SDL_CONTROLLER_BUTTON_INVALID,
		"A", SDL_CONTROLLER_BUTTON_A,
		"B", SDL_CONTROLLER_BUTTON_B,
		"X", SDL_CONTROLLER_BUTTON_X,
		"Y", SDL_CONTROLLER_BUTTON_Y,
		"Back", SDL_CONTROLLER_BUTTON_BACK,
		"Guide", SDL_CONTROLLER_BUTTON_GUIDE,
		"Start", SDL_CONTROLLER_BUTTON_START,
		"LeftStick", SDL_CONTROLLER_BUTTON_LEFTSTICK,
		"RightStick", SDL_CONTROLLER_BUTTON_RIGHTSTICK,
		"LeftShoulder", SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
		"RightShoulder", SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
		"DpadUp", SDL_CONTROLLER_BUTTON_DPAD_UP,
		"DpadDown", SDL_CONTROLLER_BUTTON_DPAD_DOWN,
		"DpadLeft", SDL_CONTROLLER_BUTTON_DPAD_LEFT,
		"DpadRight", SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
		"Misc1", SDL_CONTROLLER_BUTTON_MISC1, 
		"Paddle1", SDL_CONTROLLER_BUTTON_PADDLE1,
		"Paddle2", SDL_CONTROLLER_BUTTON_PADDLE2,
		"Paddle3", SDL_CONTROLLER_BUTTON_PADDLE3,
		"Paddle4", SDL_CONTROLLER_BUTTON_PADDLE4,
		"Touchpad", SDL_CONTROLLER_BUTTON_TOUCHPAD,
		"Max", SDL_CONTROLLER_BUTTON_MAX
	);
}

//template <> static void RegisterLuaUserType<Collider::Profile>(sol::state& lua)
//{
//	lua.new_enum("Collider::Profile",
//		"Solid", Collider::Profile::Solid,
//		"NonSolid", Collider::Profile::NonSolid,
//		"Static", Collider::Profile::Static,
//		"Dynamic", Collider::Profile::Dynamic,
//		"ApplyScale", Collider::Profile::ApplyScale
//	);
//}


//template <> static void RegisterLuaUserType<Physics>(sol::state& lua)
//{
//	lua.new_usertype<Physics>("Physics", "velocity", &Physics::velocity,
//		"acceleration", &Physics::acceleration, "mass", &Physics::mass, 
//		"drag", &Physics::drag, "gravity", &Physics::gravity, "forces", &Physics::forces);
//}
//template <> static void RegisterLuaUserType<Transform>(sol::state& lua)
//{
//	lua.new_usertype<Transform>("Transform", "scale", &Transform::scale,
//		"rotation", &Transform::rotation, "offset", &Transform::offset);
//}
//template <> static void RegisterLuaUserType<Collider>(sol::state& lua)
//{
//	lua.new_usertype<Collider>("Collider", "position", &Collider::position,
//		"dimensions", &Collider::dimensions, "material", &Collider::material, "profile", &Collider::profile);
//}

template <> static void RegisterLuaUserType<Parent>(sol::state& lua)
{
	lua.new_usertype<Parent>("Parent", "parentEntity", &Parent::entityId);
}
//template <> static void RegisterLuaUserType<Children>(sol::state& lua)
//{
//	lua.new_usertype<Children>("Children", "childEntities", &Children::childEntities);
//}

// GAME CONTROLLER
template <> static void RegisterLuaUserType<AxisInputData>(sol::state& lua)
{
	lua.new_usertype<AxisInputData>("AxisInputState", 
		"value", &AxisInputData::value, "timestamp", &AxisInputData::timestamp, 
		"state", &AxisInputData::state, "stateDuration", &AxisInputData ::stateDuration);
}
template <> static void RegisterLuaUserType<ButtonInputData>(sol::state& lua)
{
	lua.new_usertype<ButtonInputData>("ButtonInputState", 
		"button", &ButtonInputData::button, "timestamp", &ButtonInputData::timestamp,
		"state", &ButtonInputData::state, "stateDuration", &ButtonInputData::stateDuration);
}
template <> static void RegisterLuaUserType<HandedPair<AxisInputData>>(sol::state& lua)
{
	lua.new_usertype<HandedPair<AxisInputData>>("HandedPair<AxisInputState>", 
		"left", &HandedPair<AxisInputData>::left, "right", &HandedPair<AxisInputData>::right);
}
template <> static void RegisterLuaUserType<GameControllerState>(sol::state& lua)
{
	lua.new_usertype<GameControllerState>("GameControllerState", 
		"joystickID", &GameControllerState::joystickID, "axisInput", &GameControllerState::axisInput,
		"buttonInput", &GameControllerState::buttonInput);
}

// RENDERABLE
template <> static void RegisterLuaUserType<Renderable::Text::Alignment>(sol::state& lua)
{
	using Alignment = Renderable::Text::Alignment;
	lua.new_enum("Renderable::Text::Alignment", 
		"Left", Alignment::Left, "Right", Alignment::Right, "Center", Alignment::Center);
}
template <> static void RegisterLuaUserType<Renderable::Text>(sol::state& lua)
{
	using Text = Renderable::Text;
	lua.new_usertype<Text>("Renderable::Text", "sourceAtlas", &Renderable::Text::sourceAtlas,
		"text", &Text::text, "align", &Text::align, "scaleToFit", &Text::scaleToFit);
}
template <> static void RegisterLuaUserType<Renderable::Sprite>(sol::state& lua)
{
	using Sprite = Renderable::Sprite;
	lua.new_usertype<Sprite>("Renderable::Sprite", "sourceAtlas", &Renderable::Sprite::sourceAtlas,
		"seriesName", &Sprite::seriesName, "currentIndex", &Sprite::currentIndex);
}

//template <> static void RegisterLuaUserType<Handle<B2Body>>(sol::state& lua)
//{
//
//}

template <> static void RegisterLuaUserType<B2Shape::Type>(sol::state& lua)
{
	lua.new_enum("B2ShapeType",
		"Invalid", B2Shape::Type::Invalid,
		"Circle", B2Shape::Type::Circle,
		"Capsule", B2Shape::Type::Capsule,
		"Segment", B2Shape::Type::Segment,
		"Polygon", B2Shape::Type::Polygon,
		"ChainSegment", B2Shape::Type::ChainSegment
	);
}

template <> static void RegisterLuaUserType<B2Shape>(sol::state& lua)
{
	lua.new_usertype<B2Shape>("B2Shape",
		sol::constructors<B2Shape(), B2Shape(const Handle<B2Shape>&)>(),

		// Operators
		sol::meta_function::equal_to, &B2Shape::operator==,

		// Methods using same names as C++
		"GetShapeType", &B2Shape::GetShapeType,
		"GetHandle", &B2Shape::GetHandle,
		"GetParentBodyHandle", &B2Shape::GetParentBodyHandle,
		"IsValid", &B2Shape::IsValid,
		"Destroy", &B2Shape::Destroy,
		"GetDensity", &B2Shape::GetDensity,
		"SetDensity", &B2Shape::SetDensity,
		"GetFriction", &B2Shape::GetFriction,
		"SetFriction", &B2Shape::SetFriction,
		"GetRestitution", &B2Shape::GetRestitution,
		"SetRestitution", &B2Shape::SetRestitution,
		"GetBoundingBox", &B2Shape::GetBoundingBox,
		"IsPointInside", &B2Shape::IsPointInside
	);
}

template <> static void RegisterLuaUserType<B2Body::Type>(sol::state& lua)
{
	lua.new_enum("B2BodyType",
		"Static", B2Body::Type::Static,
		"Kinematic", B2Body::Type::Kinematic,
		"Dynamic", B2Body::Type::Dynamic
	);
}

template <> static void RegisterLuaUserType<B2Body>(sol::state& lua)
{
	lua.new_usertype<B2Body>("B2Body",
		// Constructor
		sol::constructors<B2Body(), B2Body(const Handle<B2Body>&)>(),

		// Equality operator
		sol::meta_function::equal_to, &B2Body::operator==,

		// Core methods
		"Destroy", &B2Body::Destroy,
		"IsValid", &B2Body::IsValid,
		"GetHandle", &B2Body::GetHandle,

		// Body Type
		"GetBodyType", &B2Body::GetBodyType,
		"SetBodyType", &B2Body::SetBodyType,

		// Rotation
		"SetFixedRotation", &B2Body::SetFixedRotation,
		"IsFixedRotation", &B2Body::IsFixedRotation,

		// Awake state
		"SetAwake", &B2Body::SetAwake,
		"IsAwake", &B2Body::IsAwake,

		// Position and angle
		"GetPosition", &B2Body::GetPosition,
		"SetPosition", &B2Body::SetPosition,
		"GetAngle", &B2Body::GetAngle,
		"SetAngle", &B2Body::SetAngle,

		// Velocity
		"GetLinearVelocity", &B2Body::GetLinearVelocity,
		"SetLinearVelocity", &B2Body::SetLinearVelocity,
		"GetAngularVelocity", &B2Body::GetAngularVelocity,
		"SetAngularVelocity", &B2Body::SetAngularVelocity,

		// Forces and impulses
		"ApplyForce", &B2Body::ApplyForce,
		"ApplyForceToCenter", &B2Body::ApplyForceToCenter,
		"ApplyLinearImpulse", &B2Body::ApplyLinearImpulse,
		"ApplyLinearImpulseToCenter", &B2Body::ApplyLinearImpulseToCenter,

		// Distance
		"GetDistance", sol::overload(
			static_cast<float (B2Body::*)(const B2Body&) const>(&B2Body::GetDistance),
			static_cast<float (B2Body::*)(SDL_FPoint) const>(&B2Body::GetDistance)
		)
	);
}



