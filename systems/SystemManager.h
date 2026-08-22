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
		requires (!SomeSystem<T> && ImplementsSystemUpdate<T> && std::constructible_from<T, Args...>)
	T& RegisterSystem(Phase ph, Args&&...args)
	{
		static_assert(std::same_as<T, std::remove_cvref_t<T>>,
			"System type template argument should have no cv-ref qualifiers");

		return userSystems_.RegisterSystem<T>(ph, std::forward<Args>(args)...);
	}

	template <typename T, typename...Args>
		requires (!SomeSystem<T> && !ImplementsSystemUpdate<T> && std::constructible_from<T, Args...>)
	T& RegisterSystem(Args&&...args)
	{
		static_assert(std::same_as<T, std::remove_cvref_t<T>>,
			"System type template argument should have no cv-ref qualifiers");

		return userSystems_.RegisterSystem<T>(std::forward<Args>(args)...);
	}

	//Result<ScriptTable::TableId> RegisterSystemScript(const std::string& filepath, Phase phase)
	//{
	//	if (!IsSystemRegistered<ScriptSystem>())
	//	{
	//		return MAKE_ERROR("ScriptSystem is not registered");
	//	}

	//	auto& scriptSys = GetSystem<ScriptSystem>();

	//	TRY(scriptSys.AddTable(filepath), tableId);

	//	auto& userScripts = scriptSys.GetScriptableUserSubSystem();

	//	auto addResult = 
	//		userScripts.AddInstance(scriptSys.GetScriptTableMap().at(tableId).table, phase);
	//	if (!addResult.Success())
	//	{
	//		scriptSys.RemoveTable(tableId);

	//		return addResult.GetError();
	//	}


	//}

	template <typename T> requires (!SomeSystem<T>)
	bool RemoveSystem()
	{
		static_assert(std::same_as<T, std::remove_cvref_t<T>>,
			"System type template argument should have no cv-ref qualifiers");

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
			return userSystems_.IsSystemRegistered<T>();
		}
	}

	UserSystemScheduler& GetUserSystemScheduler()
	{
		return userSystems_;
	}

	void ClearUserSystems()
	{
		userSystems_.Reset();
	}

	void RunSystemUpdates(Phase ph, float dt)
	{
		userSystems_.UpdateSystems(ph, dt);
	}

private:
	CoreSystemManager coreSystems_;
	UserSystemScheduler userSystems_;
};
