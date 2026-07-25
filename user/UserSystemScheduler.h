#pragma once
#include <vector>
#include <memory>
#include "../core/Algorithms.h"
#include "../core/SizedEnumMap.h"
#include "../systems/Phase.h"
#include "../systems/System.h"

//// TODO: Think about letting users register multiple update per phase functions on systems
// ex. RegisterSystemUpdate<&UserSys::UpdateMovement>(Phase::Intent)
//class UserSystemScheduler
//{
//private:
//	using SystemTypeId = uint32_t;
//
//	static SystemTypeId NextSystemTypeId()
//	{
//		static SystemTypeId id = 0;
//		return id++;
//	}
//
//	template <typename T>
//	static SystemTypeId GetSystemTypeId()
//	{
//		static SystemTypeId id = NextSystemTypeId();
//		return id;
//	}
//
//public:
//	UserSystemScheduler() = default;
//	~UserSystemScheduler() = default;
//
//	using UpdateFn = void(*)(void*, float);
//
//	struct UpdateSlot
//	{
//		Phase phase = static_cast<Phase>(-1);
//		void* instance = nullptr;
//		UpdateFn updateFn = nullptr;
//	};
// 
//	template <ImplementsSystemUpdate T, typename...Args>
//		requires std::constructible_from<T, Args...>
//	T& RegisterSystem(Phase phase, Args&&...args)
//	{
//		assert(systems_.size() == updateSlots_.size());
//
//		const auto id = static_cast<size_t>(GetSystemTypeId<T>());
//		if (id >= systems_.size())
//		{
//			systems_.resize(id + 1);
//			updateSlots_.resize(id + 1);
//		}
//		if (systems_[id] != nullptr)
//		{
//			// hard fail on re-registering for a different phase
//			assert(updateSlots_[id].phase == phase);
//
//			assert(updateSlots_[id].instance);
//			assert(updateSlots_[id].updateFn);
//
//			// return if already registered
//			return static_cast<SystemWrapper<T>&>(*systems_[id]).value;
//		}
//
//		systems_[id] = std::make_unique<SystemWrapper<T>>(std::forward<Args>(args)...);
//
//		auto& sys = static_cast<SystemWrapper<T>&>(*systems_[id]).value;
//
//		updateSlots_[id] = {
//			.phase = phase,
//			.instance = &sys,
//			.updateFn = [](void* ptr, float dt) {
//				static_cast<T*>(ptr)->Update(dt);
//			}
//		};
//
//		phaseIndices_[phase].emplace_back(id);
//
//		return sys;
//	}
//
//	template <ImplementsSystemUpdate T>
//	T& GetSystem()
//	{
//		assert(systems_.size() == updateSlots_.size());
//
//		const auto id = static_cast<size_t>(GetSystemTypeId<T>());
//		assert(id < systems_.size());
//		assert(systems_[id] != nullptr);
//
//		return static_cast<SystemWrapper<T>&>(*systems_[id]).value;
//	}
//
//	template <ImplementsSystemUpdate T>
//	const T& GetSystem() const
//	{
//		assert(systems_.size() == updateSlots_.size());
//
//		const auto id = static_cast<size_t>(GetSystemTypeId<T>());
//		assert(id < systems_.size());
//		assert(systems_[id] != nullptr);
//
//		return static_cast<SystemWrapper<T>&>(*systems_[id]).value;
//	}
//
//	template <ImplementsSystemUpdate T>
//	bool RemoveSystem()
//	{
//		assert(systems_.size() == updateSlots_.size());
//
//		const auto id = static_cast<size_t>(GetSystemTypeId<T>());
//
//		if (id >= systems_.size() || !systems_[id])
//		{
//			return false;
//		}
//
//		const auto phase = updateSlots_[id].phase;
//
//		updateSlots_[id] = {};
//		systems_[id] = nullptr;
//
//		const bool erased = core::Erase(phaseIndices_[phase], id);
//		assert(erased);
//
//		return true;
//	}
//
//	template <ImplementsSystemUpdate T>
//	bool IsSystemRegistered() const
//	{
//		assert(systems_.size() == updateSlots_.size());
//
//		const auto id = static_cast<size_t>(GetSystemTypeId<T>());
//
//		return id < systems_.size() && systems_[id] != nullptr;
//	}
//
//	void UpdateSystems(Phase phase, float dt);
//
//private:
//	SizedEnumMap<Phase, std::vector<size_t>> phaseIndices_; // phase to system ids
//	std::vector<std::unique_ptr<ISystem>> systems_;
//	std::vector<UpdateSlot> updateSlots_;
//};

namespace detail {

template <typename Fn> struct update_operation_traits;

template <typename Class, typename Ret, typename...Args>
struct update_operation_traits<Ret(Class::*)(Args...)>
{
	using system_type = Class;
	using ret_type = Ret;
	using arg_types = std::tuple<Args...>;
};

} // detail

template <typename Fn>
using update_operation_traits_t = detail::update_operation_traits<Fn>;

template <typename Fn>
concept ValidSystemUpdateOperation = requires { 
	typename update_operation_traits_t<Fn>::system_type; 
	std::same_as<typename update_operation_traits_t<Fn>::ret_type, void>;
	std::tuple_size_v<typename update_operation_traits_t<Fn>::arg_types> == 1;
	std::same_as<std::tuple_element_t<0, typename update_operation_traits_t<Fn>::arg_types>, float>;
};

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

	template <ValidSystemUpdateOperation Fn>
	static size_t ExtractUpdateOperationSystemId()
	{
		using SysType = typename update_operation_traits_t<Fn>::system_type;

		return static_cast<size_t>(GetSystemTypeId<SysType>());
	}

public:
	UserSystemScheduler() = default;
	~UserSystemScheduler() = default;

	struct SlotKey
	{
		SystemTypeId systemId = std::numeric_limits<SystemTypeId>::max();
		size_t operationIndex = std::numeric_limits<size_t>::max();
		constexpr bool operator==(const SlotKey&) const = default;
	};

	struct UpdateOperation
	{
		Phase phase = static_cast<Phase>(-1);
		void(*fn)(void*, float) = nullptr;
	};

	using UpdateOperationList = std::vector<UpdateOperation>;

	struct Slot
	{
		void* instance = nullptr;
		UpdateOperationList operationList;
	};

	template <ImplementsSystemUpdate T, typename...Args>
		requires std::constructible_from<T, Args...>
	T& RegisterSystem(Phase phase, Args&&...args)
	{
		assert(systems_.size() == slots_.size());

		const auto sysId = static_cast<size_t>(GetSystemTypeId<T>());
		if (sysId >= systems_.size())
		{
			systems_.resize(sysId + 1);
			slots_.resize(sysId + 1);
		}
		if (systems_[sysId] != nullptr)
		{
			// return if already registered
			return static_cast<SystemWrapper<T>&>(*systems_[sysId]).value;
		}

		systems_[sysId] = std::make_unique<SystemWrapper<T>>(std::forward<Args>(args)...);

		auto& sys = static_cast<SystemWrapper<T>&>(*systems_[sysId]).value;

		slots_[sysId] = {
			.instance = &sys,
			.operationList = {UpdateOperation{
				.phase = phase,
				.fn = [](void* ptr, float dt) {
					static_cast<T*>(ptr)->Update(dt);
				}}
			}
		};

		phaseToSlotKeys_[phase].emplace_back(sysId, 0);

		return sys;
	}

	template <typename T, typename...Args>
		requires (!ImplementsSystemUpdate<T> && std::constructible_from<T, Args...>)
	T& RegisterSystem(Args&&...args)
	{
		assert(systems_.size() == slots_.size());

		const auto sysId = static_cast<size_t>(GetSystemTypeId<T>());
		if (sysId >= systems_.size())
		{
			systems_.resize(sysId + 1);
			slots_.resize(sysId + 1);
		}
		if (systems_[sysId] != nullptr)
		{
			// return if already registered
			return static_cast<SystemWrapper<T>&>(*systems_[sysId]).value;
		}

		systems_[sysId] = std::make_unique<SystemWrapper<T>>(std::forward<Args>(args)...);

		auto& sys = static_cast<SystemWrapper<T>&>(*systems_[sysId]).value;

		slots_[sysId] = { .instance = &sys };

		return sys;
	}

	template <auto fn>
	void RegisterUpdateOperation(Phase phase)
	{
		using FnType = std::remove_cvref_t<decltype(fn)>;

		static_assert(ValidSystemUpdateOperation<FnType>, 
			"Function type did not match required signature 'void (Class::*)(float)'");

		using SysType = typename update_operation_traits_t<FnType>::system_type;

		const auto sysId = ExtractUpdateOperationSystemId<FnType>();
		assert(sysId < systems_.size() && "System not registered");
		assert(systems_[sysId] != nullptr && "System was null");

		auto& slot = slots_[sysId];
		const size_t opIdx = slot.operationList.size();

		slot.operationList.emplace_back(
			phase,
			[](void* ptr, float dt) { (static_cast<SysType*>(ptr)->*fn)(dt); }
		);

		phaseToSlotKeys_[phase].emplace_back(sysId, opIdx);
	}

	template <typename T>
	T& GetSystem()
	{
		assert(systems_.size() == slots_.size());

		const auto sysId = static_cast<size_t>(GetSystemTypeId<T>());
		assert(sysId < systems_.size() && "System not registered");
		assert(systems_[sysId] != nullptr && "System was null");

		return static_cast<SystemWrapper<T>&>(*systems_[sysId]).value;
	}

	template <typename T>
	const T& GetSystem() const
	{
		assert(systems_.size() == slots_.size());

		const auto sysId = static_cast<size_t>(GetSystemTypeId<T>());
		assert(sysId < systems_.size() && "System not registered");
		assert(systems_[sysId] != nullptr && "System was null");

		return static_cast<SystemWrapper<T>&>(*systems_[sysId]).value;
	}

	template <typename T>
	bool RemoveSystem()
	{
		assert(systems_.size() == slots_.size());

		const auto sysId = static_cast<size_t>(GetSystemTypeId<T>());

		if (sysId >= systems_.size() || !systems_[sysId])
		{
			return false;
		}

		auto key = SlotKey{ .systemId = sysId, .operationIndex = 0 };
		
		const auto& slot = slots_[sysId];
		for (size_t i = 0; i < slot.operationList.size(); ++i)
		{
			const auto phase = slot.operationList[i].phase;
			auto& keysForPhase = phaseToSlotKeys_[phase];

			const bool erased = core::Erase(keysForPhase, key);
			assert(erased);

			++key.operationIndex;
		}

		slots_[sysId] = {};
		systems_[sysId].reset();

		return true;
	}

	template <typename T>
	bool IsSystemRegistered() const
	{
		assert(systems_.size() == slots_.size());

		const auto sysId = static_cast<size_t>(GetSystemTypeId<T>());

		return sysId < systems_.size() && systems_[sysId] != nullptr;
	}

	void Reset();

	void UpdateSystems(Phase phase, float dt);

private:
	SizedEnumMap<Phase, std::vector<SlotKey>> phaseToSlotKeys_;
	std::vector<std::unique_ptr<ISystem>> systems_;
	std::vector<Slot> slots_;
};