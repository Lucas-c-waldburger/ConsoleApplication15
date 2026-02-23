#pragma once
#include "System.h"
#include "../core/Result.h"
#include "SystemRegistry.h"
#include <memory>
#include "../core/TypeUtils.h"

template <typename T>
concept SomeSystem = std::derived_from<T, System> &&
					 SomeTypeInList<T, SystemTypeList>;

class CoreSystemManager
{
public:
	CoreSystemManager() = default;
	~CoreSystemManager() = default;

	template <SomeSystem T>
	T& GetSystem()
	{
		auto& sys = std::get<std::unique_ptr<T>>(systems_);
		assert(sys);

		return *sys;
	}

	template <SomeSystem T>
	const T& GetSystem() const
	{
		const auto& sys = std::get<std::unique_ptr<T>>(systems_);
		assert(sys);

		return *sys;
	}

	template <SomeSystem T, typename...Args>
		requires std::constructible_from<T, Args...>
	T& RegisterSystem(Args&&...args)
	{
		auto& sys = std::get<std::unique_ptr<T>>(systems_);
		if (!sys)
		{
			sys = std::make_unique<T>(std::forward<Args>(args)...);
		}

		return *sys;
	}

	template <SomeSystem T>
	bool IsSystemRegistered() const
	{
		return std::get<std::unique_ptr<T>>(systems_) != nullptr;
	}

private:
	SystemTypeList::AsTuple<std::unique_ptr> systems_;
};