#pragma once
#include <vector>
#include <tuple>
#include "TypeUtils.h"

template <typename TList>
class TypeListVectorMap;

template <typename...Ts>
class TypeListVectorMap<TypeList<Ts...>>
{
public:
	template <SomeTypeInPack<Ts...> T>
	auto& GetEntry() { return std::get<std::vector<T>>(entries_); }

	template <SomeTypeInPack<Ts...> T>
	const auto& GetEntry() const { return std::get<std::vector<T>>(entries_); }

	template <typename Fn>
	void ForEachEntry(Fn&& fn)
	{
		((fn(GetEntry<Ts>())), ...);
	}

private:
	std::tuple<std::vector<Ts>...> entries_;
};