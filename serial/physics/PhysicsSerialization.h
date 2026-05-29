#pragma once
#include "../../deps/nlohmann/json.hpp"	
#include "../../core/FixedString.h"



//template <typename T, FixedString name, auto serializeFn = nullptr, auto deserializeFn = nullptr>
//struct Serializer
//{
//	static void Serialize(nlohmann::json& j, const T& t) requires HasToBasicJson<T>
//	{
//		to_json(j[name], t);
//	}
//	static void Serialize(nlohmann::ordered_json& j, const T& t) requires HasToOrderedJson<T>
//	{
//		to_json(j[name], t);
//	}
//
//	template <typename...Args> requires (!HasToBasicJson<T>&&
//		std::invocable<decltype(serializeFn), nlohmann::json&, const T&, Args...>)
//	static void Serialize(nlohmann::json& j, const T& t, Args&&...args)
//	{
//		std::invoke(serializeFn, j[name], t, std::forward<Args>(args)...);
//	}
//
//	template <typename...Args> requires (!HasToOrderedJson<T> && 
//		std::invocable<decltype(serializeFn), nlohmann::ordered_json&, const T&, Args...>)
//	static void Serialize(nlohmann::ordered_json& j, const T& t, Args&&...args)
//	{
//		std::invoke(serializeFn, j[name], t, std::forward<Args>(args)...);
//	}
//
//	static void Deserialize(const nlohmann::json& j, T& t) requires HasFromBasicJson<T>
//	{
//		from_json(j[name], t);
//	}
//	static void Deserialize(const nlohmann::ordered_json& j, T& t) requires HasFromOrderedJson<T>
//	{
//		from_json(j[name], t);
//	}
//
//	template <typename...Args> requires (!HasFromBasicJson<T>&&
//		std::invocable<decltype(deserializeFn), const nlohmann::json&, T&, Args...>)
//	static void Deserialize(nlohmann::json& j, T& t, Args&&...args)
//	{
//		std::invoke(deserializeFn, j[name], t, std::forward<Args>(args)...);
//	}
//
//	template <typename...Args> requires (!HasFromOrderedJson<T>&&
//		std::invocable<decltype(deserializeFn), const nlohmann::ordered_json&, T&, Args...>)
//	static void Deserialize(nlohmann::ordered_json& j, T& t, Args&&...args)
//	{
//		std::invoke(deserializeFn, j[name], t, std::forward<Args>(args)...);
//	}
//};