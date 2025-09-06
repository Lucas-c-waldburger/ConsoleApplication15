#pragma once
#include "EventSignal.h"
#include "ControllerInputSignal.h"

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
		storageVec.emplace_back(std::forward<T>(event));
		heldEventIndices_[heldEventHead_++] = T::eventType;
	}

	void Dispatch(EventSignalList& eventSignals, ControllerInputSignalList& inputSignals)
	{
		for (size_t i = 0; i < heldEventHead_; i++)
		{
			dispatchTable_[heldEventIndices_[i]](storage_, eventSignals, inputSignals, true);
		}

		heldEventHead_ = 0;
	}

	void Discard(EventSignalList& eventSignals, ControllerInputSignalList& inputSignals)
	{
		for (size_t i = 0; i < heldEventHead_; i++)
		{
			dispatchTable_[heldEventIndices_[i]](storage_, eventSignals, inputSignals, false);
		}

		heldEventHead_ = 0;
	}

	template <SomeEventData T>
	const auto& Peek()
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
	using FnType = void(*)(StorageTuple&, EventSignalList&, bool);

	static inline constexpr std::array<FnType, N> dispatchTable_ = {
		(+[](StorageTuple& store, EventSignalList& eventSignals, 
			 ControllerInputSignalList& inputSignals, bool dispatch)
		{
			auto& storageVec = std::get<std::vector<Ts>>(store);

			if (dispatch) 
			{
				for (auto& event : storageVec)
				{
					eventSignals.Emit(event);

					if constexpr (std::same_as<Ts, events::GameControllerInput>)
					{
						inputSignals.Emit(event);
					}
				}
			}

			storageVec.clear();
		})...
	};
	 
	StorageTuple storage_;
	std::array<size_t, N> heldEventIndices_;
	size_t heldEventHead_;
};


using EventStorage = EventStorageImpl<EventDataTypeList>;