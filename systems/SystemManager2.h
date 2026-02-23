#pragma once
#include "System.h"
#include "SystemRegistry.h"
#include "SystemManager.h"
#include "../core/SizedEnumMap.h"
#include "Phase.h"
#include <vector>
#include <memory>


//namespace detail {
//
//template <typename Tup>
//struct core_system_tuple;
//
//template <template <typename...> class Tup, typename...Ts>
//struct core_system_tuple<Tup<Ts...>>
//{
//	using type = std::tuple<std::unique_ptr<Ts>...>;
//};
//
//template <typename...Ts>
//struct type_holder { using type = TypeList<Ts...>; };
//
//} // detail
//
//using core_system_tuple_t = typename detail::core_system_tuple<SystemTypeList>::type;
//
//template <typename...Ts>
//using type_holder_t = typename detail::type_holder<Ts...>::type;
//
//using SetupPhaseSystems = type_holder_t<TimerSystem>;
//using InputPhaseSystems = type_holder_t<SDLInputSystem>;
//using IntentPhaseSystems = type_holder_t<>;
//using SimulationPhaseSystems = type_holder_t<PhysicsSystem>;
//using SimResponsePhaseSystems = type_holder_t<AudioSystem, CameraSystem>;
//using RenderPrepPhaseSystems = type_holder_t<SpriteAnimationSystem,
//											 NewRenderSystem>;
//using CleanupPhaseSystems = type_holder_t<>;
//
//namespace detail {
//
//template <Phase ph> struct systems_for_phase;
//template <> struct systems_for_phase<Phase::Setup> : SetupPhaseSystems {};
//template <> struct systems_for_phase<Phase::Input> : InputPhaseSystems {};
//template <> struct systems_for_phase<Phase::Intent> : IntentPhaseSystems {};
//template <> struct systems_for_phase<Phase::Simulation> : SimulationPhaseSystems {};
//template <> struct systems_for_phase<Phase::SimResponse> : SimResponsePhaseSystems {};
//template <> struct systems_for_phase<Phase::RenderPrep> : RenderPrepPhaseSystems {};
//template <> struct systems_for_phase<Phase::Cleanup> : CleanupPhaseSystems {};
//
//} // detail
//
//template <Phase ph> 
//using systems_for_phase_t = typename detail::systems_for_phase<ph>::type;
//
//template <Phase ph>
//concept PhaseMappedToSystemList = requires {
//	typename systems_for_phase_t<ph>;
//};
//
//class SystemCoordinator
//{
//private:
//	template <typename Tup>
//	struct update_systems_impl;
//
//	template <template <typename...> class Tup, typename...Ts>
//	struct update_systems_impl<Tup<Ts...>>
//	{
//		static void call(core_system_tuple_t& coreSystems,
//						 UserSystemScheduler& userSystems, 
//						 Phase ph, float dt)
//		{
//			static constexpr auto updateCore = []<typename T>(auto& cSystems, float d) {
//				auto& sys = std::get<std::unique_ptr<T>>(cSystems);
//				if (sys)
//				{
//					sys->Update(d);
//				}
//			};
//
//			((updateCore.template operator()<Ts>(coreSystems, dt)), ...);
//			userSystems.UpdateSystems(ph, dt);
//		}	
//	};
//
//public:
//	template <Phase ph>
//	void UpdateSystems(float dt)
//	{
//		static_assert(PhaseMappedToSystemList<ph>,
//			"Phase argument not mapped to any system list");
//
//		update_systems_impl<systems_for_phase_t<ph>>::call(
//			coreSystems_, userSystems_, ph, dt);
//	}
//
//	template <typename T, typename...Args> requires std::constructible_from<T, Args...>
//	T& RegisterSystem(Phase phase, Args&&...args)
//	{
//		static_assert(std::same_as<T, std::remove_cvref_t<T>>,
//			"System type argument should have no cv-ref qualifiers");
//
//		static_assert(ImplementsSystemUpdate<T>, "System type must "
//			"implement method 'void Update(float)'");
//
//		return userSystems_.RegisterSystem<T>(phase, std::forward<Args>(args)...);
//	}
//
//	template <typename T, typename...Args>
//		requires (SomeSystem<std::remove_cvref_t<T>> &&
//				  std::constructible_from<T, Args...>)
//	T& RegisterSystem(Args&&...args)
//	{
//		static_assert(std::same_as<T, std::remove_cvref_t<T>>,
//			"System type argument should have no cv-ref qualifiers");
//
//		auto& sys = std::get<std::unique_ptr<T>>(coreSystems_);
//		sys = std::make_unique<T>(std::forward<Args>(args)...);
//
//		return *sys;
//	}
//
//	template <typename T>
//	T& GetSystem() 
//	{ 
//		static_assert(std::same_as<T, std::remove_cvref_t<T>>,
//			"System type argument should have no cv-ref qualifiers");
//
//		if constexpr (SomeSystem<T>)
//		{
//			auto& sys = std::get<std::unique_ptr<T>>(coreSystems_);
//			assert(sys);
//
//			return *sys;
//		}
//		else
//		{
//			static_assert(ImplementsSystemUpdate<T>, "System type must "
//				"implement method 'void Update(float)', "
//				"and therefore must not be registered");
//
//			return userSystems_.GetSystem<T>();
//		}
//	}
//	template <typename T>
//	const T& GetSystem() const 
//	{ 
//		static_assert(std::same_as<T, std::remove_cvref_t<T>>,
//			"System type argument should have no cv-ref qualifiers");
//
//		if constexpr (SomeSystem<T>)
//		{
//			auto& sys = std::get<std::unique_ptr<T>>(coreSystems_);
//			assert(sys);
//
//			return *sys;
//		}
//		else
//		{
//			static_assert(ImplementsSystemUpdate<T>, "System type must "
//				"implement method 'void Update(float)', "
//				"and therefore must not be registered");
//
//			return userSystems_.GetSystem<T>();
//		}
//	}	
//
//	template <typename T>
//	bool IsSystemRegistered() const
//	{
//		static_assert(std::same_as<T, std::remove_cvref_t<T>>,
//			"System type argument should have no cv-ref qualifiers");
//
//		if constexpr (SomeSystem<T>)
//		{
//			return std::get<std::unique_ptr<T>>(coreSystems_) != nullptr;
//		}
//		else  
//		{
//			static_assert(ImplementsSystemUpdate<T>, "System type must "
//				"implement method 'void Update(float)', "
//				"and therefore must not be registered");
//
//			return userSystems_.IsSystemRegistered<T>();
//		}
//	}
//
//private:
//	core_system_tuple_t coreSystems_;
//	UserSystemScheduler userSystems_;
//};

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