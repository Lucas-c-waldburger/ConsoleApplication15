#pragma once
#include <vector>
#include <memory>
#include "../core/Algorithms.h"
#include "../core/SizedEnumMap.h"
#include "../systems/Phase.h"
#include "../systems/System.h"

class UserSystemScheduler
{
private:
	using SystemTypeId = uint32_t;

	static SystemTypeId NextSystemTypeId()
	{
		static SystemTypeId id = 0;
		return id++;
	}

	template <typename T>
	static SystemTypeId GetSystemTypeId()
	{
		static SystemTypeId id = NextSystemTypeId();
		return id;
	}

public:
	UserSystemScheduler() = default;
	~UserSystemScheduler() = default;

	using UpdateFn = void(*)(void*, float);

	struct UpdateSlot
	{
		Phase phase = static_cast<Phase>(-1);
		void* instance = nullptr;
		UpdateFn updateFn = nullptr;
	};

	template <ImplementsSystemUpdate T, typename...Args>
		requires std::constructible_from<T, Args...>
	T& RegisterSystem(Phase phase, Args&&...args)
	{
		//assert(systems_.size() == updateSlots_.size());

		//const auto id = static_cast<size_t>(GetSystemTypeId<T>());
		//if (id >= typeIdToDataIndex_.size())
		//{
		//	typeIdToDataIndex_.resize(id + 1, std::numeric_limits<size_t>::max());
		//}
		//if (typeIdToDataIndex_[id] != std::numeric_limits<size_t>::max())
		//{
		//	assert(typeIdToDataIndex_[id] < systems_.size());

		//	return static_cast<SystemWrapper<T>&>(
		//		*systems_[typeIdToDataIndex_[id]]).value;
		//}

		//typeIdToDataIndex_[id] = systems_.size();

		//systems_emplace_back(
		//	std::make_unique<SystemWrapper<T>>(std::forward<Args>(args)...)
		//);

		//auto& sys = static_cast<SystemWrapper<T>&>(*systems_.back()).value;

		//updateSlots_.emplace_back({
		//	.phase = phase,
		//	.instance = &sys,
		//	.updateFn = [](void* ptr, float dt) {
		//		static_cast<T*>(ptr)->Update(dt);
		//	}
		//});

		//phaseIndices_[phase].emplace_back(typeIdToDataIndex_[id]);
		//
		//return sys;
		// 
		assert(systems_.size() == updateSlots_.size());

		const auto id = static_cast<size_t>(GetSystemTypeId<T>());
		if (id >= systems_.size())
		{
			systems_.resize(id + 1);
			updateSlots_.resize(id + 1);
		}
		if (systems_[id] != nullptr)
		{
			// hard fail on re-registering for a different phase
			assert(updateSlots_[id].phase == phase);

			assert(updateSlots_[id].instance);
			assert(updateSlots_[id].updateFn);

			// return if already registered
			return static_cast<SystemWrapper<T>&>(*systems_[id]).value;
		}

		systems_[id] = std::make_unique<SystemWrapper<T>>(std::forward<Args>(args)...);

		auto& sys = static_cast<SystemWrapper<T>&>(*systems_[id]).value;

		updateSlots_[id] = {
			.phase = phase,
			.instance = &sys,
			.updateFn = [](void* ptr, float dt) {
				static_cast<T*>(ptr)->Update(dt);
			}
		};

		phaseIndices_[phase].emplace_back(id);

		return sys;
	}

	template <ImplementsSystemUpdate T>
	T& GetSystem()
	{
		assert(systems_.size() == updateSlots_.size());

		//const auto id = static_cast<size_t>(GetSystemTypeId<T>());
		//assert(id < typeIdToDataIndex_.size());

		//const size_t dataIdx = typeIdToDataIndex_[id];
		//assert(dataIdx < systems_.size());
		//assert(systems_[dataIdx] != nullptr);
		//assert(updateSlots_[dataIdx].instance != nullptr);
		//assert(updateSlots_[dataIdx].updateFn != nullptr);

		//return static_cast<SystemWrapper<T>&>(*systems_[dataIdx]).value;
		const auto id = static_cast<size_t>(GetSystemTypeId<T>());
		assert(id < systems_.size());
		assert(systems_[id] != nullptr);

		return static_cast<SystemWrapper<T>&>(*systems_[id]).value;
	}

	template <ImplementsSystemUpdate T>
	const T& GetSystem() const
	{
		assert(systems_.size() == updateSlots_.size());

		//const auto id = static_cast<size_t>(GetSystemTypeId<T>());
		//assert(id < typeIdToDataIndex_.size());

		//const size_t dataIdx = typeIdToDataIndex_[id];
		//assert(dataIdx < systems_.size());
		//assert(systems_[dataIdx] != nullptr);
		//assert(updateSlots_[dataIdx].instance != nullptr);
		//assert(updateSlots_[dataIdx].updateFn != nullptr);

		//return static_cast<SystemWrapper<T>&>(*systems_[dataIdx]).value;
		const auto id = static_cast<size_t>(GetSystemTypeId<T>());
		assert(id < systems_.size());
		assert(systems_[id] != nullptr);

		return static_cast<SystemWrapper<T>&>(*systems_[id]).value;
	}

	template <ImplementsSystemUpdate T>
	bool RemoveSystem()
	{
		assert(systems_.size() == updateSlots_.size());

		//if (systems_.empty())
		//{
		//	return false;
		//}

		//const auto id = static_cast<size_t>(GetSystemTypeId<T>());
		//if (id >= typeIdToDataIndex_.size() ||
		//	typeIdToDataIndex_[id] == std::numeric_limits<size_t>::max())
		//{
		//	return false;
		//}

		//const size_t idxToErase = typeIdToDataIndex_[id];
		//const size_t backIdx = systems_.size() - 1;
		//assert(idxToErase < systems_.size());

		//Phase phaseToEraseIdxFrom = updateSlots_[idxToErase].phase;

		//const bool removedPhaseIdx = core::Erase(
		//	phaseIndices_[phaseToEraseIdxFrom], idxToErase
		//);
		//assert(removedPhaseIdx);

		//if (idxToErase != backIdx)
		//{
		//	// change idx ref inside phase to new pos of back idx system
		//	Phase phaseToUpdateIdxOf = updateSlots_[backIdx].phase;
		//	auto phasesIdxIt = core::Find(
		//		phaseIndices_[phaseToUpdateIdxOf], backIdx
		//	);
		//	assert(phasesIdxIt != phaseIndices_[phaseToUpdateIdxOf].end());

		//	*phasesIdxIt = idxToErase;

		//	// find the typeId of back idx system as an index
		//	auto backDataTypeIdSlot = core::Find(typeIdToDataIndex_, backIdx);
		//	assert(backDataTypeIdSlot != typeIdToDataIndex_.end());

		//	// update the system vec index for the typeId to point to its new location
		//	*backDataTypeIdSlot = idxToErase;
		//	typeIdToDataIndex_[id] = std::numeric_limits<size_t>::max();

		//	std::swap(systems_[idxToErase], systems_[backIdx]);
		//	std::swap(updateSlots_[idxToErase], updateSlots_[backIdx]);
		//}

		//systems_.pop_back();
		//updateSlots_.pop_back();

		//return true;
		const auto id = static_cast<size_t>(GetSystemTypeId<T>());

		if (id >= systems_.size() || !systems_[id])
		{
			return false;
		}

		const auto phase = updateSlots_[id].phase;

		updateSlots_[id] = {};
		systems_[id] = nullptr;

		const bool erased = core::Erase(phaseIndices_[phase], id);
		assert(erased);

		return true;
	}

	template <ImplementsSystemUpdate T>
	bool IsSystemRegistered() const
	{
		assert(systems_.size() == updateSlots_.size());

		//const auto id = static_cast<size_t>(GetSystemTypeId<T>());

		//if (id >= typeIdToDataIndex_.size() ||
		//	typeIdToDataIndex_[id] == std::numeric_limits<size_t>::max())
		//{
		//	return false;
		//}

		//assert(typeIdToDataIndex_[id] < systems_.size());
		//assert(systems_[typeIdToDataIndex_[id]] != nullptr);
		//assert(updateSlots_[typeIdToDataIndex_[id]].instance != nullptr);
		//assert(updateSlots_[typeIdToDataIndex_[id]].updateFn != nullptr);

		//return true;

		const auto id = static_cast<size_t>(GetSystemTypeId<T>());

		return id < systems_.size() && systems_[id] != nullptr;
	}

	void UpdateSystems(Phase phase, float dt);

private:
	SizedEnumMap<Phase, std::vector<size_t>> phaseIndices_;
	std::vector<std::unique_ptr<ISystem>> systems_;
	std::vector<UpdateSlot> updateSlots_;
	//std::vector<size_t> typeIdToDataIndex_;
};