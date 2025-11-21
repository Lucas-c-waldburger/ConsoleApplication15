#pragma once
#include "../../deps/nlohmann/json.hpp"
#include "../../core/commonObjects.h"
#include "../../core/Anchor.h"

template <typename T>
inline void to_json(nlohmann::json& j, const HandedPair<T>& p)
{
	j["left"] = p.left;
	j["right"] = p.right;
}
template <typename T>
inline void from_json(const nlohmann::json& j, HandedPair<T>& p)
{
	j.at("left").get_to(p.left);
	j.at("right").get_to(p.right);
}


template <typename T>
inline void to_json(nlohmann::json& j, const Dimensions<T>& d)
{
	j["w"] = d.w;
	j["h"] = d.h;
}
template <typename T>
inline void from_json(const nlohmann::json& j, Dimensions<T>& d)
{
	j.at("w").get_to(d.w);
	j.at("h").get_to(d.h);
}

NLOHMANN_JSON_SERIALIZE_ENUM(
	Anchor,
	{
		{Anchor::Left,		  "Left"},
		{Anchor::Right,		  "Right"},
		{Anchor::Top,		  "Top"},
		{Anchor::Bottom,	  "Bottom"},
		{Anchor::CenterX,     "CenterX"},
		{Anchor::CenterY,     "CenterY"},
		{Anchor::TopLeft,     "TopLeft"},
		{Anchor::TopRight,    "TopRight"},
		{Anchor::BottomLeft,  "BottomLeft"},
		{Anchor::BottomRight, "BottomRight"},
		{Anchor::Center,	  "Center"}
	}
)

