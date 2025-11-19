#pragma once
#include "System.h"
#include "../core/Result.h"
#include "SystemRegistry.h"
#include <memory>
#include "../core/TypeUtils.h"

template <typename T>
concept SomeSystem = std::derived_from<T, System> &&
					 SomeTypeInList<T, SystemTypeList>;

namespace detail {
template <typename TList>
struct system_tuple;

template <typename...Ts>
struct system_tuple<TypeList<Ts...>>
{
	using type = std::tuple<std::unique_ptr<Ts>...>;
};
}

using system_tuple_t = detail::system_tuple<SystemTypeList>::type;

class SystemManager
{
public:
	SystemManager() = default;
	~SystemManager() = default;

	template <SomeSystem T>
	std::unique_ptr<T>& GetSystem()
	{
		return std::get<std::unique_ptr<T>>(systems_);
	}

	template <SomeSystem T>
	const std::unique_ptr<T>& GetSystem() const
	{
		return std::get<std::unique_ptr<T>>(systems_);
	}

	template <SomeSystem T, typename...Args> requires (std::constructible_from<T, Args...>)
	std::unique_ptr<T>& InitializeSystem(Args&&...args)
	{
		auto& sys = GetSystem<T>();
		if (!sys)
		{
			sys = std::make_unique<T>(std::forward<Args>(args)...);
		}

		return sys;
	}

	template <SomeSystem T>
	bool IsSystemInitialized() const
	{
		return GetSystem<T>() != nullptr;
	}

private:
	system_tuple_t systems_;
};


//namespace impl {
//	using SystemManager = SystemManagerTemplate<SYSTEM_REGISTRY_LIST>;
//}