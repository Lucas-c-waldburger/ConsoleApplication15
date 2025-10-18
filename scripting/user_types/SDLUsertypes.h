#pragma once
#include "../LuaUserType.h"
#include <SDL_rect.h>

DEF_LUA_USERTYPE(SDL_Point) {
	lua.def_type(sol::constructors<SDL_Point(), SDL_Point(int, int)>(), 
				 "x", &SDL_Point::x, "y", &SDL_Point::y);
}
DEF_LUA_USERTYPE(SDL_FPoint) {
	lua.def_type(sol::constructors<SDL_FPoint(), SDL_FPoint(float, float)>(),
				 "x", &SDL_FPoint::x, "y", &SDL_FPoint::y);
}
DEF_LUA_USERTYPE(SDL_Rect) {
	lua.def_type(sol::constructors<SDL_Rect(), SDL_Rect(int, int, int, int)>(),
				 "x", &SDL_Rect::x, "y", &SDL_Rect::y, "w", &SDL_Rect::w, "h", &SDL_Rect::h);
}
DEF_LUA_USERTYPE(SDL_FRect) {
	lua.def_type(sol::constructors<SDL_Rect(), SDL_Rect(float, float, float, float)>(),
				 "x", &SDL_FRect::x, "y", &SDL_FRect::y, "w", &SDL_FRect::w, "h", &SDL_FRect::h);
}