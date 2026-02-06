#pragma once
#include "System.h"
#include "SystemRegistry.h"
#include "SystemManager.h"
#include "../core/SizedEnumMap.h"
#include <vector>
#include <memory>

enum class Phase
{
	Setup = 0,
	Input,
	Intent,
	Simulation,
	SimResponse,
	RenderPrep,
	Cleanup,
	ENUM_SIZE_
};
static_assert(SomeSizedEnum<Phase>);

//CameraSystem,
//PhysicsSystem,
//SDLInputSystem,
//TimerSystem,
//SpriteAnimationSystem,
//EntityStateSystem,
//GameLoopSystem,
//AudioSystem,
//NewRenderSystem,
//SerializationSystem

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
		void* instance = nullptr;
		UpdateFn updateFn = nullptr;
	};

	template <ImplementsSystemUpdate T, typename...Args>
		requires std::constructible_from<T, Args...>
	T& RegisterSystem(Phase phase, Args&&...args)
	{
		const auto id = static_cast<size_t>(GetSystemTypeId<T>());

		assert(systems_.size() == updateSlots_.size());
		if (id >= systems_.size()) 
		{
			systems_.resize(id + 1);
			updateSlots_.resize(id + 1);
		}
		if (systems_[id] != nullptr)
		{
			assert(updateSlots_[id].instance);
			assert(updateSlots_[id].updateFn);
			// return if already registered
			return static_cast<SystemWrapper<T>&>(*systems_[id]).value;
		}

		systems_[id] = std::make_unique<SystemWrapper<T>>(std::forward<Args>(args)...);

		auto& sys = static_cast<SystemWrapper<T>&>(*systems_[id]).value;

		updateSlots_[id] = {
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
		const auto id = static_cast<size_t>(GetSystemTypeId<T>());
		assert(id < systems_.size());
		assert(systems_[id] != nullptr);

		return static_cast<SystemWrapper<T>&>(*systems_[id]).value;
	}

	template <ImplementsSystemUpdate T>
	const T& GetSystem() const
	{
		const auto id = static_cast<size_t>(GetSystemTypeId<T>());
		assert(id < systems_.size());
		assert(systems_[id] != nullptr);

		return static_cast<SystemWrapper<T>&>(*systems_[id]).value;
	}

	template <ImplementsSystemUpdate T>
	bool IsSystemRegistered() const
	{
		const auto id = static_cast<size_t>(GetSystemTypeId<T>());

		return id < systems_.size() && systems_[id] != nullptr;
	}

	void UpdateSystems(Phase phase, float dt)
	{
		for (const size_t idx : phaseIndices_[phase])
		{
			assert(idx < updateSlots_.size());
			
			auto& slot = updateSlots_[idx];
			assert(slot.updateFn);
			assert(slot.instance);

			std::invoke(slot.updateFn, slot.instance, dt);
		}
	}

private:
	SizedEnumMap<Phase, std::vector<size_t>> phaseIndices_;
	std::vector<std::unique_ptr<ISystem>> systems_;
	std::vector<UpdateSlot> updateSlots_;
};

namespace detail {

template <typename Tup>
struct core_system_tuple;

template <template <typename...> class Tup, typename...Ts>
struct core_system_tuple<Tup<Ts...>>
{
	using type = std::tuple<std::unique_ptr<Ts>...>;
};

template <typename...Ts>
struct type_holder { using type = TypeList<Ts...>; };

} // detail

using core_system_tuple_t = typename detail::core_system_tuple<SystemTypeList>::type;

template <typename...Ts>
using type_holder_t = typename detail::type_holder<Ts...>::type;

using SetupPhaseSystems = type_holder_t<TimerSystem>;
using InputPhaseSystems = type_holder_t<SDLInputSystem>;
using IntentPhaseSystems = type_holder_t<>;
using SimulationPhaseSystems = type_holder_t<PhysicsSystem>;
using SimResponsePhaseSystems = type_holder_t<AudioSystem, CameraSystem>;
using RenderPrepPhaseSystems = type_holder_t<SpriteAnimationSystem,
											 NewRenderSystem>;
using CleanupPhaseSystems = type_holder_t<>;

namespace detail {

template <Phase ph> struct systems_for_phase;
template <> struct systems_for_phase<Phase::Setup> : SetupPhaseSystems {};
template <> struct systems_for_phase<Phase::Input> : InputPhaseSystems {};
template <> struct systems_for_phase<Phase::Intent> : IntentPhaseSystems {};
template <> struct systems_for_phase<Phase::Simulation> : SimulationPhaseSystems {};
template <> struct systems_for_phase<Phase::SimResponse> : SimResponsePhaseSystems {};
template <> struct systems_for_phase<Phase::RenderPrep> : RenderPrepPhaseSystems {};
template <> struct systems_for_phase<Phase::Cleanup> : CleanupPhaseSystems {};

} // detail

template <Phase ph> 
using systems_for_phase_t = typename detail::systems_for_phase<ph>::type;

template <Phase ph>
concept PhaseMappedToSystemList = requires {
	typename systems_for_phase_t<ph>;
};

class SystemCoordinator
{
private:
	template <typename Tup>
	struct update_systems_impl;

	template <template <typename...> class Tup, typename...Ts>
	struct update_systems_impl<Tup<Ts...>>
	{
		static void call(core_system_tuple_t& coreSystems,
						 UserSystemScheduler& userSystems, 
						 Phase ph, float dt)
		{
			static constexpr auto updateCore = []<typename T>(auto& cSystems, float d) {
				auto& sys = std::get<std::unique_ptr<T>>(cSystems);
				if (sys)
				{
					sys->Update(d);
				}
			};

			((updateCore.template operator()<Ts>(coreSystems, dt)), ...);
			userSystems.UpdateSystems(ph, dt);
		}	
	};

public:
	template <Phase ph>
	void UpdateSystems(float dt)
	{
		static_assert(PhaseMappedToSystemList<ph>,
			"Phase argument not mapped to any system list");

		update_systems_impl<systems_for_phase_t<ph>>::call(
			coreSystems_, userSystems_, ph, dt);
	}

	template <typename T, typename...Args> requires std::constructible_from<T, Args...>
	T& RegisterSystem(Phase phase, Args&&...args)
	{
		static_assert(std::same_as<T, std::remove_cvref_t<T>>,
			"System type argument should have no cv-ref qualifiers");

		static_assert(ImplementsSystemUpdate<T>, "System type must "
			"implement method 'void Update(float)'");

		return userSystems_.RegisterSystem<T>(phase, std::forward<Args>(args)...);
	}

	template <typename T, typename...Args>
		requires (SomeSystem<std::remove_cvref_t<T>> &&
				  std::constructible_from<T, Args...>)
	T& RegisterSystem(Args&&...args)
	{
		static_assert(std::same_as<T, std::remove_cvref_t<T>>,
			"System type argument should have no cv-ref qualifiers");

		auto& sys = std::get<std::unique_ptr<T>>(coreSystems_);
		sys = std::make_unique<T>(std::forward<Args>(args)...);

		return *sys;
	}

	template <typename T>
	T& GetSystem() 
	{ 
		static_assert(std::same_as<T, std::remove_cvref_t<T>>,
			"System type argument should have no cv-ref qualifiers");

		if constexpr (SomeSystem<T>)
		{
			auto& sys = std::get<std::unique_ptr<T>>(coreSystems_);
			assert(sys);

			return *sys;
		}
		else
		{
			static_assert(ImplementsSystemUpdate<T>, "System type must "
				"implement method 'void Update(float)', "
				"and therefore must not be registered");

			return userSystems_.GetSystem<T>();
		}
	}
	template <typename T>
	const T& GetSystem() const 
	{ 
		static_assert(std::same_as<T, std::remove_cvref_t<T>>,
			"System type argument should have no cv-ref qualifiers");

		if constexpr (SomeSystem<T>)
		{
			auto& sys = std::get<std::unique_ptr<T>>(coreSystems_);
			assert(sys);

			return *sys;
		}
		else
		{
			static_assert(ImplementsSystemUpdate<T>, "System type must "
				"implement method 'void Update(float)', "
				"and therefore must not be registered");

			return userSystems_.GetSystem<T>();
		}
	}	

	template <typename T>
	bool IsSystemRegistered() const
	{
		static_assert(std::same_as<T, std::remove_cvref_t<T>>,
			"System type argument should have no cv-ref qualifiers");

		if constexpr (SomeSystem<T>)
		{
			return std::get<std::unique_ptr<T>>(coreSystems_) != nullptr;
		}
		else  
		{
			static_assert(ImplementsSystemUpdate<T>, "System type must "
				"implement method 'void Update(float)', "
				"and therefore must not be registered");

			return userSystems_.IsSystemRegistered<T>();
		}
	}

private:
	core_system_tuple_t coreSystems_;
	UserSystemScheduler userSystems_;
};

//template <typename SysList>
//class SystemPhaseCoordinatorImpl;
//
//template <template <typename...> class SysList, typename...Ts>
//class SystemPhaseCoordinatorImpl<SysList<Ts...>>
//{
//public:
//
//private:
//	Phase phase_;
//	std::tuple<Ts...> coreSystems_;
//	std::vector<size_t> scheduledUserSystemIndices_;
//};