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

	j["id"] = e.GetID();

	//auto& cmps = j["components"];
	nlohmann::ordered_json cmps;
	cmps = nlohmann::ordered_json::object();

	auto impl = [&]<typename T> {
		if (e.HasComponent<T>())
		{
			auto& obj = cmps[ComponentName<T>::value];

			to_json(obj, e.GetComponent<T>());
		}
	};

	SerializableComponentTypeList::ForEachType(impl);

	j["components"] = std::move(cmps);
}


//inline void from_json(const nlohmann::json& j, Entity& e)
//{
//	if (!e.IsValid())
//	{
//		LOG_ERROR("Cannot deserialize into an invalid entity.");
//		return;
//	}
//
//	auto impl = [&]<typename T>
//	{
//		if (j.contains(ComponentTypeToName<T>::value))
//		{
//			T cmp{};
//
//			try {
//				cmp = j.at(ComponentTypeToName<T>::value).get<T>();
//			}
//			catch (const nlohmann::json::exception& ex) {
//				DeserializationReport::PushComponentError<T>(ex);
//			}
//
//			e.AddComponent(std::move(cmp));
//		}	
//	};
//
//	SerializableComponentTypeList::ForEachType(impl);
//}

template <JsonSerializableComponent T>
inline Error MakeComponentDeserializationError(const nlohmann::json::exception& err)
{
	static constexpr std::string_view errFmt = "Could not deserialize component '{}': {}";

	return MAKE_ERROR_FMT(errFmt, ComponentName<T>::value, err.what());
}

inline void from_json(const nlohmann::json& j, ComponentDeserializationTarget& target)
{
	auto impl = [&]<typename T>
	{
		constexpr std::string_view componentName = ComponentName<T>::value;

		using Field = ComponentDeserializationTargetWrapper<T>;

		Field& field = std::get<Field>(target);

		if (j.contains(componentName))
		{
			try 
			{
				field = Field{ std::make_optional(j.at(componentName).get<T>()) };
			}
			catch (const nlohmann::json::exception& ex) 
			{
				field = Field{ MakeComponentDeserializationError<T>(ex) };
			}
		}
	};

	SerializableComponentTypeList::ForEachType(impl);
}