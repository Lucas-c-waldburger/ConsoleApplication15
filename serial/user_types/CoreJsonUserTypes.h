#pragma once
#include "../../deps/nlohmann/json.hpp"
#include "../../core/commonObjects.h"
#include "../../core/Literals.h"
#include "../../core/Handle.h"
#include "../../core/Anchor.h"
#include "../../core/Dictionary.h"

template <typename BasicJson, typename T>
inline void to_json(BasicJson& j, const HandedPair<T>& p)
{
	j["left"] = p.left;
	j["right"] = p.right;
}
template <typename BasicJson, typename T>
inline void from_json(const BasicJson& j, HandedPair<T>& p)
{
	j.at("left").get_to(p.left);
	j.at("right").get_to(p.right);
}

template <typename BasicJson, typename T>
inline void to_json(BasicJson& j, const Dimensions<T>& d)
{
	j["w"] = d.w;
	j["h"] = d.h;
}
template <typename BasicJson, typename T>
inline void from_json(const BasicJson& j, Dimensions<T>& d)
{
	j.at("w").get_to(d.w);
	j.at("h").get_to(d.h);
}

template <typename BasicJson, typename T>
inline void to_json(BasicJson& j, const Range<T>& r)
{
	j["min"] = r.min;
	j["max"] = r.max;
}
template <typename BasicJson, typename T>
inline void from_json(const BasicJson& j, Range<T>& r)
{
	j.at("min").get_to(r.min);
	j.at("max").get_to(r.max);
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

/* HANDLE */
template <typename BasicJson, typename T>
inline void to_json(BasicJson& j, const Handle<T>& handle)
{
	if (handle.IsValid())
	{
		j["hash"] = handle.GetHash();
	}
	else
	{
		j["hash"] = nullptr;
	}	
}
template <typename BasicJson, typename T>
inline void from_json(const BasicJson& j, Handle<T>& handle)
{}

/* DICTIONARY */
template <
	template <typename, typename, typename...> class MapType,
	typename Value, typename... Ts,
	typename BasicJson
>
void to_json(BasicJson& j, const DictionaryTemplate<MapType, Value, Ts...>& dict)
{
	j = BasicJson::object();
	for (const auto& [key, val] : dict) 
	{
		j[key] = val;
	}
}

template <
	template <typename, typename, typename...> class MapType,
	typename Value, typename... Ts,
	typename BasicJson
>
void from_json(const BasicJson& j, DictionaryTemplate<MapType, Value, Ts...>& dict)
{
	dict.clear();
	if (!j.is_object()) 
	{
		return;
	}

	for (auto it = j.begin(); it != j.end(); ++it) 
	{
		dict[it.key()] = it.value().template get<Value>();
	}
}
