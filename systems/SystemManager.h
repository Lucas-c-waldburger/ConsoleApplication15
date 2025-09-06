#pragma once
#include "System.h"
#include "../core/Result.h"
#include "SystemRegistry.h"
#include <memory>
#include "../core/TypeUtils.h"

//class SystemManager
//{
//public:
//	template <SomeSystem T, typename...Args>
//	Result<Void> AddSystem(Args&&...args)
//	{
//		std::type_index typeIdx = typeid(T);
//
//		if (registry_.contains(typeIdx))
//		{
//			return MAKE_ERROR("Registry already contains instance of this system");
//		}
//
//		registry_[typeIdx] = MakeTypeErased(new T(std::forward<Args>(args)...));
//
//		return Void{};
//	}
//
//	template <SomeSystem T>
//	bool RemoveSystem()
//	{
//		return static_cast<bool>(registry_.erase(typeid(T)));
//	}
//
//	template <SomeSystem T>
//	T* GetSystem()
//	{
//		std::type_index typeIdx = typeid(T);
//
//		auto it = registry_.find(typeIdx);
//		if (it == registry_.end())
//		{
//			return nullptr;
//		}
//
//		return static_cast<T*>(it->second.get());
//	}
//
//	template <SomeSystem T>
//	bool IsRegistered() const { return registry_.contains(typeid(T)); }
//
//private:
//	using TypeErasedSystem = std::unique_ptr<void, void(*)(void*)>;
//
//	template <SomeSystem T>
//	static TypeErasedSystem MakeTypeErased(T* ptr) 
//	{
//		return { static_cast<void*>(ptr), [](void* p) { delete static_cast<T*>(p); } };
//	}
//
//	std::unordered_map<std::type_index, TypeErasedSystem> registry_;
//};


template <typename...Ts> requires pack_types_unique_v<Ts...>
class SystemManagerTemplate
{
public:
	SystemManagerTemplate() = default;
	~SystemManagerTemplate() = default;

	template <SomeTypeInPack<Ts...> T>
	std::unique_ptr<T>& GetSystem()
	{
		return std::get<std::unique_ptr<T>>(systems_);
	}

	template <SomeTypeInPack<Ts...> T>
	const std::unique_ptr<T>& GetSystem() const
	{
		return std::get<std::unique_ptr<T>>(systems_);
	}

	template <SomeTypeInPack<Ts...> T, typename...Args>
	std::unique_ptr<T>& InitializeSystem(Args&&...args)
	{
		auto& sys = GetSystem<T>();
		if (!sys)
		{
			sys = std::make_unique<T>(std::forward<Args>(args)...);
		}

		return sys;
	}

	template <SomeTypeInPack<Ts...> T>
	bool IsSystemInitialized() const
	{
		return GetSystem<T>() != nullptr;
	}

	bool AllSystemsInitialized() const
	{
		return (IsSystemInitialized<Ts>() && ...);
	}

private:
	std::tuple<std::unique_ptr<Ts>...> systems_;
};


namespace impl {
	using SystemManager = SystemManagerTemplate<SYSTEM_REGISTRY_LIST>;
}