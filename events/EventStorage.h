#pragma once
#include "EventSignal.h"
#include "ControllerInputSignal.h"
#include "Event.h"
#include "EventDataTypeList.h"
#include "data/EventDataIncludes.h"
#include "../inputs/SignalListCollection.h"

template <typename TList>
class EventStorageImpl;

template <SomeEventData...Ts>
class EventStorageImpl<TypeList<Ts...>>
{
public:
	static constexpr size_t N = sizeof...(Ts);

	EventStorageImpl() : heldEventIndices_(FillEventIndicesArray()), heldEventHead_(0) {}

	template <SomeEventData T>
	void Emplace(T&& event)
	{
		auto& storageVec = std::get<std::vector<T>>(storage_);
		if (storageVec.empty())
		{
			// need to mark that this event type has at least 1 event to be dispatched
			heldEventIndices_[heldEventHead_++] = T::eventType;
		}

		storageVec.emplace_back(std::forward<T>(event));

		//auto& storageVec = std::get<std::vector<T>>(storage_);
		//storageVec.emplace_back(std::forward<T>(event));
		//heldEventIndices_[heldEventHead_++] = T::eventType;
	}

	template <SomeEventData T>
	void EmplaceRange(std::vector<T>&& events)
	{
		if (events.empty())
		{
			return;
		}

		auto& storageVec = std::get<std::vector<T>>(storage_);
		if (storageVec.empty())
		{
			heldEventIndices_[heldEventHead_++] = T::eventType;
		}

		const size_t eventsSize = events.size();
		const size_t storageSize = storageVec.size();
		storageVec.resize(storageSize + eventsSize);

		storageVec.insert(storageVec.begin() + storageSize, 
			std::make_move_iterator(events.begin()), 
			std::make_move_iterator(events.end()));
	}

	void Dispatch(SignalListCollection& signalListCollection)
	{
		for (size_t i = 0; i < heldEventHead_; i++)
		{
			dispatchTable_[heldEventIndices_[i]](storage_, signalListCollection, true);
		}

		heldEventHead_ = 0;
	}

	//template <SomeEventData T>
	//void Discard()
	//{
	//	for (size_t i = 0; i < heldEventHead_; i++)
	//	{
	//		dispatchTable_[heldEventIndices_[i]](storage_, eventSignals, inputSignals, false);
	//	}

	//	heldEventHead_ = 0;
	//}

	void Discard()
	{
		((std::get<std::vector<Ts>>(storage_).clear()), ...);

		heldEventHead_ = 0;
	}

	template <SomeEventData T>
	const auto& Peek() const
	{
		return std::get<std::vector<T>>(storage_);
	}

	template <SomeEventData T>
	size_t NumEvents() const { return std::get<std::vector<T>>(storage_).size(); }

private:
	static constexpr auto FillEventIndicesArray()
	{
		std::array<size_t, N> arr{};
		arr.fill(kInvalidEventType);
		return arr;
	}

	using StorageTuple = std::tuple<std::vector<Ts>...>;
	using FnType = void(*)(StorageTuple&, SignalListCollection&, bool);

	static inline constexpr std::array<FnType, N> dispatchTable_ = {
		(+[](StorageTuple& store, SignalListCollection& signalListCollection,
			 bool dispatch)
		{
			auto& storageVec = std::get<std::vector<Ts>>(store);

			if (dispatch) 
			{
				for (auto& event : storageVec)
				{
					signalListCollection.Emit(event);
				}
			}

			storageVec.clear();
		})...
	};
	 
	StorageTuple storage_;
	std::array<size_t, N> heldEventIndices_; // keep track which types actually hold events
	size_t heldEventHead_;
};


using EventStorage = EventStorageImpl<EventDataTypeList>;