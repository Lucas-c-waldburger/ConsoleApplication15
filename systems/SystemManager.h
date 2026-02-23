#pragma once
#include "CoreSystemManager.h"
#include "../user/UserSystemScheduler.h"


class SystemManager
{
public:
	SystemManager() = default;
	~SystemManager() = default;

	template <typename T>
	T& GetSystem()
	{
		static_assert(std::same_as<T, std::remove_cvref_t<T>>,
			"System type template argument should have no cv-ref qualifiers");

		if constexpr (SomeSystem<T>)
		{
			return coreSystems_.GetSystem<T>();
		}
		else
		{
			static_assert(ImplementsSystemUpdate<T>, 
				"User system must implement public method 'void Update(float)'");
			return userSystems_.GetSystem<T>();
		}
	}

	template <typename T>
	const T& GetSystem() const
	{
		static_assert(std::same_as<T, std::remove_cvref_t<T>>,
			"System type template argument should have no cv-ref qualifiers");

		if constexpr (SomeSystem<T>)
		{
			return coreSystems_.GetSystem<T>();
		}
		else
		{
			static_assert(ImplementsSystemUpdate<T>,
				"User system must implement public method 'void Update(float)'");
			return userSystems_.GetSystem<T>();
		}
	}

	template <SomeSystem T, typename...Args> 
		requires std::constructible_from<T, Args...>
	T& RegisterSystem(Args&&...args)
	{
		static_assert(std::same_as<T, std::remove_cvref_t<T>>,
			"System type template argument should have no cv-ref qualifiers");

		return coreSystems_.RegisterSystem<T>(std::forward<Args>(args)...);
	}

	template <typename T, typename...Args>
		requires (!SomeSystem<T> && std::constructible_from<T, Args...>)
	T& RegisterSystem(Phase ph, Args&&...args)
	{
		static_assert(std::same_as<T, std::remove_cvref_t<T>>,
			"System type template argument should have no cv-ref qualifiers");

		static_assert(ImplementsSystemUpdate<T>,
			"User system must implement public method 'void Update(float)'");

		return userSystems_.RegisterSystem<T>(ph, std::forward<Args>(args)...);
	}

	template <typename T> requires (!SomeSystem<T>)
	bool RemoveSystem()
	{
		static_assert(std::same_as<T, std::remove_cvref_t<T>>,
			"System type template argument should have no cv-ref qualifiers");

		static_assert(ImplementsSystemUpdate<T>,
			"User system must implement public method 'void Update(float)'");

		return userSystems_.RemoveSystem<T>();
	}

	template <typename T>
	bool IsSystemRegistered() const
	{
		static_assert(std::same_as<T, std::remove_cvref_t<T>>,
			"System type template argument should have no cv-ref qualifiers");

		if constexpr (SomeSystem<T>)
		{
			return coreSystems_.IsSystemRegistered<T>();
		}
		else
		{
			static_assert(ImplementsSystemUpdate<T>,
				"User system must implement public method 'void Update(float)'");
			return userSystems_.IsSystemRegistered<T>();
		}
	}

	void RunSystemUpdates(Phase ph, float dt)
	{
		userSystems_.UpdateSystems(ph, dt);
	}

private:
	CoreSystemManager coreSystems_;
	UserSystemScheduler userSystems_;
};
