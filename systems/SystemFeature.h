#pragma once
#include "System.h"
#include "../core/Result.h"
#include "SystemRegistry.h"
#include "../core/TypeUtils.h"
#include "AudioSystem.h"

class Entity;

//template <SomeSystem Derived>
//class IEntityDestroyedSystem
//{
//public:
//	void EntityDestroyed(Entity& entity)
//	{
//		return static_cast<Derived*>(this)->EntityDestroyedImpl(entity);
//	}
//};
//
//template <typename T>
//concept SomeEntityDestroyedSystem = 
//	SomeSystem<T> && std::derived_from<T, IEntityDestroyedSystem<T>>;
//
//
//
//template <SomeSystem Derived>
//class IEntityDriverProviderSystem
//{
//public:
//	template <typename Driver>
//	Result<Driver> GetEntityDriver(Entity& entity)
//	{
//		return static_cast<Derived*>(this)->GetEntityDriverImpl(entity);
//	}
//};
//
//template <typename T>
//concept SomeEntityDriverProviderSystem =
//	SomeSystem<T> && std::derived_from<T, IEntityDriverProviderSystem<T>>;
//
//
//template <typename T>
//concept SomeEntityDriverProviderSystem =
//SomeSystem<T> && std::derived_from<T, IEntityDriverProviderSystem<T>>;
//
//
//template <SomeSystem Derived>
//class ICleanupSystem
//{
//public:
//	Result<Void> Cleanup()
//	{
//		return static_cast<Derived*>(this)->CleanupImpl();
//	}
//};
//
//template <typename T>
//concept SomeCleanupSystem =
//	SomeSystem<T> && std::derived_from<T, ICleanupSystem<T>>;
//
//



namespace detail {
// SYSTEM TUPLE
template <typename TList>
struct system_tuple;

template <typename...Ts>
struct system_tuple<TypeList<Ts...>>
{
	using type = std::tuple<std::unique_ptr<Ts>...>;
};

// TYPE IN SYSTEM TUPLE
template <typename T, typename Tuple>
struct type_in_system_tuple;

template <typename T, typename...Ts>
struct type_in_system_tuple<T, std::tuple<std::unique_ptr<Ts>...>>
{
	static constexpr bool value = (std::same_as<T, Ts> || ...);
};
} // detail


class SystemRegistry
{
private:
	using SystemTuple = detail::system_tuple<SystemTypeList>::type;

	template <typename T>
	static constexpr bool type_in_system_tuple_v =
		type_in_tuple_v<std::unique_ptr<T>, SystemTuple>;

public:
	template <typename T> requires type_in_system_tuple_v<T>
	static const std::unique_ptr<T>& GetSystem()
	{
		return GetInstance().GetSystemInternal<T>();
	}

	template <typename T, typename...Args> 
		requires (type_in_system_tuple_v<T> && std::constructible_from<T, Args...>)
	static const std::unique_ptr<T>& InitSystem(Args&&...args)
	{
		auto& system = GetInstance().GetSystemInternal<T>();

		system = std::make_unique<T>(std::forward<Args>(args)...);

		return system;
	}

	template <typename T> requires type_in_system_tuple_v<T>
	static bool IsInitialized()
	{
		return GetInstance().GetSystemInternal<T>() != nullptr;
	}

private:
	SystemRegistry& GetInstance()
	{
		static std::unique_ptr<SystemRegistry> instance;
		if (!instance)
		{
			instance = std::unique_ptr<SystemRegistry>(new SystemRegistry());
		}

		return *instance;
	}

	template <typename T> requires type_in_system_tuple_v<T>
	auto& GetSystemInternal()
	{
		return std::get<std::unique_ptr<T>>(systems_);
	}
	template <typename T> requires type_in_system_tuple_v<T>
	const auto& GetSystemInternal()
	{
		return std::get<std::unique_ptr<T>>(systems_);
	}

	SystemTuple systems_;
};