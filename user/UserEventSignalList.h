#pragma once
#include <array>
#include <unordered_map>
#include <tuple>
#include "../core/Signal.h"
#include "../core/TypeInfo.h"
#include "../events/EventSignalConcepts.h"
#include "../events/EventDataTypeList.h"
#include "../events/data/EventDataIncludes.h"
#include "UserEventUtils.h"

using UserEventSignal = Signal<const InlineStorage<kUserEventStorageSize>&>;

class UserEventSignalList
{
public:
	template <typename T>
	bool RegisterEvent(std::string_view name = "")
	{
		static_assert(std::same_as<std::remove_cvref_t<T>, T>,
			"template argument must have no cv-ref qualifiers");

		if (typeIdToArrayIndex_.contains(TypeInfo<T>::hash32))
		{
			LOG_INFO("Duplicate event registration");
			return false;
		}

		if (typeIdToArrayIndex_.size() >= signals_.size())
		{
			LOG_ERROR("No more user events can be registered");
			return false;
		}

		assert(nextFreeSlot_ < signals_.size());

		typeIdToArrayIndex_.try_emplace(TypeInfo<T>::hash32, nextFreeSlot_);

		eventNames_[nextFreeSlot_] = (!name.empty())
			? std::string{ name }
			: std::string{ TypeInfo<T>::name };

		++nextFreeSlot_;

		return true;
	}

	template <typename T>
	bool IsEventRegistered() const
	{
		static_assert(std::same_as<std::remove_cvref_t<T>, T>,
			"template argument must have no cv-ref qualifiers");

		auto it = typeIdToArrayIndex_.find(TypeInfo<T>::hash32);

		return it != typeIdToArrayIndex_.end() && it->second < signals_.size();
	}

	template <typename Fn>
	SignalToken Connect(Fn&& fn)
	{
		using UserData = std::remove_cvref_t<
			typename func_traits<Fn>::template arg_at<0>
		>;

		if (!IsEventRegistered<UserData>())
		{
			if (!RegisterEvent<UserData>())
			{
				return {};
			}
		}

		//const auto typeId = static_cast<size_t>(GetUserEventTypeId<UserData>());
		//if (typeId >= signals_.size())
		//{
		//	LOG_ERROR("No more user events can be registered");
		//	return {};
		//}

		auto it = typeIdToArrayIndex_.find(TypeInfo<UserData>::hash32);
		assert(it != typeIdToArrayIndex_.end());
		assert(it->second < signals_.size());

		return signals_[it->second].Connect([f = std::forward<Fn>(fn)]
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

	void Reset()
	{
		typeIdToArrayIndex_.clear();
		eventNames_.fill("");
		nextFreeSlot_ = 0;

		for (auto& signal : signals_)
		{
			signal.ClearSlots();
		}
	}

private:
	std::unordered_map<uint32_t, size_t> typeIdToArrayIndex_;
	std::array<std::string, UserEventTypeList::size> eventNames_;
	std::array<UserEventSignal, UserEventTypeList::size> signals_;
	size_t nextFreeSlot_ = 0;
};