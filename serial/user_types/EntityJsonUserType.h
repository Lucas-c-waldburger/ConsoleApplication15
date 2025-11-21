#pragma once
#include "../Serialization.h"
#include "../../ecs/Ecs.h"

inline void to_json(nlohmann::json& j, const Entity& e)
{
	if (!e.IsValid())
	{
		LOG_ERROR("Cannot serialize an invalid entity.");
		return;
	}

	auto impl = [&]<typename T> {
		if (e.HasComponent<T>())
		{
			j[ComponentTypeToName<T>::value] = e.GetComponent<T>();
		}
	};

	SerializableComponentTypeList::ForEachType(impl);
}

inline void from_json(const nlohmann::json& j, Entity& e)
{
	if (!e.IsValid())
	{
		LOG_ERROR("Cannot deserialize into an invalid entity.");
		return;
	}

	auto impl = [&]<typename T>
	{
		if (j.contains(ComponentTypeToName<T>::value))
		{
			e.AddComponent(j.at(ComponentTypeToName<T>::value).get<T>());
		}	
	};

	SerializableComponentTypeList::ForEachType(impl);
}

inline void from_json(const nlohmann::json& j, SerializableComponentTuple& tup)
{
	auto impl = [&]<typename T>
	{
		if (j.contains(ComponentTypeToName<T>::value))
		{
			std::get<T>(tup).emplace(j.at(ComponentTypeToName<T>::value).get<T>());
		}
	};

	SerializableComponentTypeList::ForEachType(impl);
}

//inline SerializableComponentTuple MakeSerializableComponentTuple(const Entity& e)
//{
//	if (!e.IsValid())
//	{
//		LOG_ERROR("Cannot make serializable component tuple for an invalid entity.");
//		return;
//	}
//
//	SerializableComponentTuple tup{};
//
//	auto impl = [&]<typename T> {
//		if constexpr (JsonSerializableComponent<T>)
//		{
//			if (e.HasComponent<T>())
//			{
//				std::get<T>(tup).emplace(e.GetComponent<T>());
//			}
//		}
//	};
//
//	ComponentTypeList::ForEachType(impl);
//
//	return tup;
//}
//
//inline void to_json(nlohmann::json& j, const SerializableComponentTuple& tup)
//{
//	auto impl = [&]<size_t I> {
//		if (std::get<I>(tup).has_value())
//		{
//
//		}
//	};
//}