#pragma once
#include "../../deps/nlohmann/json.hpp"
#include "../../core/commonObjects.h"

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