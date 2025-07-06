#pragma once
#include "../../ecs/Ecs.h"
#include "../ComponentConcepts.h"

template <typename Derived, SomeComponent...ReqComponents>
class BaseDriver
{
public:
	using Super = BaseDriver<Derived, ReqComponents...>;

	BaseDriver(const BaseDriver&) = delete;
	BaseDriver& operator=(const BaseDriver&) = delete;
	BaseDriver(BaseDriver&&) = delete;
	BaseDriver& operator=(BaseDriver&&) = delete;

	template <typename...Args>
	static Result<Derived> GetInstance(Entity& entity, Args&&...args)
	{
		if (!entity.IsValid())
		{
			return MAKE_ERROR("Entity was invalid");
		}
		if (!entity.HasComponents<ReqComponents...>())
		{
			return MAKE_ERROR("Entity did not have all required components");
		}

		return Derived{ entity, std::forward<Args>(args)... };
	}

	template <typename...Args>
	static Result<Derived> GetInstance(Entity_t id, Args&&...args)
	{
		return GetInstance(ECS::GetEntityByID(id), std::forward<Args>(args)...);
	}

protected:
	explicit BaseDriver(Entity& entity) : entity_(entity) {}

	template <SomeTypeInPack<ReqComponents...> T>
	T& GetComponent() { return entity_.GetComponent<T>(); }

	template <SomeTypeInPack<ReqComponents...> T>
	const T& GetComponent() const { return entity_.GetComponent<T>(); }

	template <SomeComponent T>
	T& AddComponent(T&& cmp) { return entity_.AddComponent(std::forward<T>(cmp)); }

	template <SomeComponent T>
	T& AddComponent() { return entity_.AddComponent<T>(); }

	Entity& GetEntity() { return entity_; }

	const Entity& GetEntity() const { return entity_; }

private:
	Entity entity_;
};
