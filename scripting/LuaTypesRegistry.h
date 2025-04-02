#pragma once
#include <sol/sol.hpp>
#include <lua.hpp>
#include <filesystem>
#include "../components/ComponentRegistry.h"
#include "../core/commonObjects.h"
#include "../core/Handle.h"
#include "ScriptInfo.h"
#include <SDL.h>

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


// HANDLES
class GlyphAtlas;
template <> static void RegisterLuaUserType<Handle<GlyphAtlas>>(sol::state& lua)
{
	lua.new_usertype<Handle<GlyphAtlas>>("Handle<GlyphAtlas>",
		sol::meta_function::equal_to, &Handle<GlyphAtlas>::operator==);

	lua["Handle<GlyphAtlas>"]["__ne"] = [](const Handle<GlyphAtlas>& lhs, const Handle<GlyphAtlas>& rhs) {
		return lhs != rhs;
	};
}
class SpriteSeriesAtlas;
template <> static void RegisterLuaUserType<Handle<SpriteSeriesAtlas>>(sol::state& lua)
{
	lua.new_usertype<Handle<SpriteSeriesAtlas>>("Handle<SpriteSeriesAtlas>",
		sol::meta_function::equal_to, &Handle<SpriteSeriesAtlas>::operator==);

	lua["Handle<SpriteSeriesAtlas>"]["__ne"] = 
		[](const Handle<SpriteSeriesAtlas>& lhs, const Handle<SpriteSeriesAtlas>& rhs) {
			return lhs != rhs;
	};
}
template <> static void RegisterLuaUserType<Force>(sol::state& lua)
{
	lua.new_usertype<Force>("Force", "vector", &Force::vector, "duration", &Force::duration);
}
template <> static void RegisterLuaUserType<AccumulatedForces>(sol::state& lua)
{
	lua.new_usertype<AccumulatedForces>("AccumulatedForces", "normed",
		&AccumulatedForces::normed, "max", &AccumulatedForces::max);
}
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

// COMPONENTS

template <> static void RegisterLuaUserType<Physics>(sol::state& lua)
{
	lua.new_usertype<Physics>("Physics", "velocity", &Physics::velocity,
		"acceleration", &Physics::acceleration, "mass", &Physics::mass, 
		"drag", &Physics::drag, "gravity", &Physics::gravity, "forces", &Physics::forces);
}
template <> static void RegisterLuaUserType<Spatial>(sol::state& lua)
{
	try {
		lua.new_usertype<Spatial>("Spatial",
			"position", &Spatial::position,
			"dimensions", &Spatial::dimensions);
	} catch (sol::error& e) { std::cout << e.what() << '\n'; }
}
template <> static void RegisterLuaUserType<Transform>(sol::state& lua)
{
	lua.new_usertype<Transform>("Transform", "scale", &Transform::scale,
		"rotation", &Transform::rotation, "offset", &Transform::offset);
}

template <> static void RegisterLuaUserType<Parent>(sol::state& lua)
{
	lua.new_usertype<Parent>("Parent", "parentEntity", &Parent::parentEntity);
}
template <> static void RegisterLuaUserType<Children>(sol::state& lua)
{
	lua.new_usertype<Children>("Children", "childEntities", &Children::childEntities);
}

// GAME CONTROLLER
template <> static void RegisterLuaUserType<AxisInputState>(sol::state& lua)
{
	lua.new_usertype<AxisInputState>("AxisInputState", 
		"value", &AxisInputState::value, "timestamp", &AxisInputState::timestamp, 
		"state", &AxisInputState::state, "stateDuration", &AxisInputState ::stateDuration);
}
template <> static void RegisterLuaUserType<ButtonInputState>(sol::state& lua)
{
	lua.new_usertype<ButtonInputState>("ButtonInputState", 
		"button", &ButtonInputState::button, "timestamp", &ButtonInputState::timestamp,
		"state", &ButtonInputState::state, "stateDuration", &ButtonInputState::stateDuration);
}
template <> static void RegisterLuaUserType<HandedPair<AxisInputState>>(sol::state& lua)
{
	lua.new_usertype<HandedPair<AxisInputState>>("HandedPair<AxisInputState>", 
		"left", &HandedPair<AxisInputState>::left, "right", &HandedPair<AxisInputState>::right);
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



//// LUA KEY
//template <typename T> struct LuaKeyTemplate;
//template <typename T> static constexpr const char* LuaKey = LuaKeyTemplate<T>::value;
//
//template <> struct LuaKeyTemplate<Spatial> { static constexpr const char* value = };