#pragma once
#include <array>
#include <unordered_map>
#include <tuple>
#include "../core/Signal.h"
#include "../events/EventSignalConcepts.h"
#include "../events/EventDataTypeList.h"
#include "../events/data/EventDataIncludes.h"
#include "UserEventUtils.h"

using UserEventSignal = Signal<const InlineStorage<kUserEventStorageSize>&>;

class UserEventSignalList
{
public:
	template <typename Fn>
	SignalToken Connect(Fn&& fn)
	{
		using UserData = std::remove_cvref_t<
			typename func_traits<Fn>::template arg_at<0>
		>;

		const auto typeId = static_cast<size_t>(GetUserEventTypeId<UserData>());
		if (typeId >= signals_.size())
		{
			LOG_ERROR("No more user events can be registered");
			return {};
		}

		return signals_[typeId].Connect([f = std::forward<Fn>(fn)]
			(const InlineStorage<kUserEventStorageSize>& data) mutable {
				std::invoke(f, data.Get<UserData>());
			});
	}

	template <SomeUserEvent T>
	void Emit(const T& ev)
	{
		static constexpr size_t evIndex = index_of_v<T, UserEventTypeList>;
		
		signals_[evIndex].Emit(ev.data);
	}

private:
	std::array<UserEventSignal, UserEventTypeList::size> signals_;
};