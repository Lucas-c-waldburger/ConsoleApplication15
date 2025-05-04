#pragma once
#include "System.h"
#include "../core/Result.h"
#include "../core/commonObjects.h"
#include <typeindex>
#include <unordered_map>
#include <memory>

class SystemRegistry
{
public:
	template <SomeSystem T, typename...Args>
	Result<Void> AddSystem(Args&&...args)
	{
		std::type_index typeIdx = typeid(T);

		if (registry_.contains(typeIdx))
		{
			return MAKE_ERROR("Registry already contains instance of this system");
		}

		registry_[typeIdx] = MakeTypeErased(new T(std::forward<Args>(args)...));

		return Void{};
	}

	template <SomeSystem T>
	bool RemoveSystem()
	{
		return static_cast<bool>(registry_.erase(typeid(T)));
	}

	template <SomeSystem T>
	T* GetSystem()
	{
		std::type_index typeIdx = typeid(T);

		auto it = registry_.find(typeIdx);
		if (it == registry_.end())
		{
			return nullptr;
		}

		return static_cast<T*>(it->second.get());
	}

	template <SomeSystem T>
	bool IsRegistered() const { return registry_.contains(typeid(T)); }

private:
	using TypeErasedSystem = std::unique_ptr<void, void(*)(void*)>;

	template <SomeSystem T>
	static TypeErasedSystem MakeTypeErased(T* ptr) 
	{
		return { static_cast<void*>(ptr), [](void* p) { delete static_cast<T*>(p); } };
	}

	std::unordered_map<std::type_index, TypeErasedSystem> registry_;
};

