#pragma once
#include "../core/TypeListVectorMap.h"
#include "EventConcepts.h"
#include "EventDataTypeList.h"
#include "data/EventDataIncludes.h"

class EventDataStorage
{
public:
	template <SomeEventData T>
	const T& Emplace(T&& event)
	{
		return storage_.GetEntry<T>().emplace_back(std::forward<T>(event));
	}

	template <SomeEventData T>
	const std::vector<T>& GetStorageEntry() const
	{
		return storage_.GetEntry<T>();
	}

	template <SomeEventData T>
	void Clear()
	{
		storage_.GetEntry<T>().clear();
	}

	void ClearAll()
	{
		storage_.ForEachEntry([](auto& entry) { entry.clear(); });
	}


private:
	TypeListVectorMap<EventDataTypeList> storage_;
};