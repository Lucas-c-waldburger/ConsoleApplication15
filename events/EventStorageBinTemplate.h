#pragma once
#include "custom/CustomEventData.h"

template <SomeCustomEvent...Ts> requires pack_types_unique_v<Ts...>
class EventDataStorageBinTemplate
{
public:
	using Types = TypeList<Ts...>;

	template <PackMemberType<Ts...> T>
	std::vector<T>& GetEvents()
	{
		return std::get<std::vector<T>>(storage_);
	}

	void Clear()
	{
		((GetEvents<Ts>().clear()), ...);
	}

private:
	std::tuple<std::vector<Ts>...> storage_;
};