#pragma once
#include <array>
#include <iostream>
#include "../deps/nlohmann/json.hpp"
#include "../ecs/Ecs.h"
#include "../components/ComponentTypeList.h"

//inline void to_json(nlohmann::json& j, const Entity& e)
//{
//	if (!e.IsValid())
//	{
//		return;
//	}
//
//	auto impl = [&]<SomeComponent T> {
//		if (e.HasComponent<T>())
//		{
//			to_json(j, e.GetComponent<T>());
//		}
//	};
//
//	ComponentTypeList::ForEachType(impl);
//}
//
//inline void from_json(const nlohmann::json& j, Entity& e)
//{
//	if (!e.IsValid())
//	{
//		return;
//	}
//
//
//}