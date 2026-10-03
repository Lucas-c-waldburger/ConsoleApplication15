#pragma once
#include "UserTypesCommon.h"
#include <SDL_rect.h>
#include <SDL_pixels.h>

DEF_REGISTER_LUA_USERTYPE(SDL_Point, "SDL_Point",
	sol::constructors<SDL_Point(), SDL_Point(int, int)>(),
	"x", &SDL_Point::x, "y", &SDL_Point::y);

DEF_REGISTER_LUA_USERTYPE(SDL_FPoint, "SDL_FPoint",
	sol::constructors<SDL_FPoint(), SDL_FPoint(float, float)>(),
		"x", &SDL_FPoint::x, "y", &SDL_FPoint::y);

DEF_REGISTER_LUA_USERTYPE(SDL_Rect, "SDL_Rect",
	sol::constructors<SDL_Rect(), SDL_Rect(int, int, int, int)>(),
	"x", &SDL_Rect::x, "y", &SDL_Rect::y, "w", &SDL_Rect::w, "h", &SDL_Rect::h);

DEF_REGISTER_LUA_USERTYPE(SDL_FRect, "SDL_FRect",
	sol::constructors<SDL_FRect(), SDL_FRect(float, float, float, float)>(),
	"x", &SDL_FRect::x, "y", &SDL_FRect::y, "w", &SDL_FRect::w, "h", &SDL_FRect::h);

DEF_REGISTER_LUA_USERTYPE(SDL_Color, "SDL_Color",
	"r", &SDL_Color::r,
	"g", &SDL_Color::g,
	"b", &SDL_Color::b,
	"a", &SDL_Color::a);