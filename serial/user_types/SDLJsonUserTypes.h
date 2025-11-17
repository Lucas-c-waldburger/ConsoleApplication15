#pragma once
#include "../../sdl/SDLUtils.h"
#include "../../deps/nlohmann/json.hpp"

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SDL_Point, x, y)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SDL_FPoint, x, y)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SDL_Rect, x, y, w, h)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SDL_FRect, x, y, w h)

//template <SDLPointType P>
//inline void to_json(nlohmann::json& j, const P& p)
//{
//	j["x"] = p.x;
//	j["y"] = p.y;
//}
//
//template <SDLPointType P>
//inline void from_json(const nlohmann::json& j, P& p)
//{
//	j.at("x").get_to(p.x);
//	j.at("y").get_to(p.y);
//}
//
//template <SDLRectType R>
//inline void to_json(nlohmann::json& j, const R& r)
//{
//	j["x"] = r.x;
//	j["y"] = r.y;
//	j["w"] = r.w;
//	j["h"] = r.h;
//}
//
//template <SDLRectType R>
//inline void from_json(const nlohmann::json& j, R& r)
//{
//	j.at("x").get_to(r.x);
//	j.at("y").get_to(r.y);
//	j.at("w").get_to(r.w);
//	j.at("h").get_to(r.h);
//}