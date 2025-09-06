#pragma once
#include <cassert>
#include <memory>
#include <unordered_map>
#include "System.h"
#include "../state/EntityState.h"
#include "../core/CatTypeID.h"
#include "../core/Result.h"
#include "../components/EntityStateComponent.h"


struct EntityStateTable;

template <typename T>
concept ImplementsOnEnter = std::is_invocable_r_v<void, typename T::OnEnter, Entity&>;

template <typename T>
concept ImplementsOnExit = std::is_invocable_r_v<void, typename T::OnExit, Entity&>; 

template <typename T>
concept ImplementsOnUpdate = std::is_invocable_r_v<void, typename T::OnUpdate, Entity&, float>;

using UniqueVoidPtr = std::unique_ptr<void, void(*)(void*)>;

template <typename T, typename...Args> requires std::constructible_from<T, Args...>
inline UniqueVoidPtr MakeUniqueVoidPtr(Args&&...args) {
	return UniqueVoidPtr(new T{ std::forward<Args>(args)... },
		[](void* p) { delete static_cast<T*>(p); });
}


template <typename Derived>
class EntityStateContract 
{
public:
	static EntityStateTable GetStateTable();
};
 
template <typename T>
concept SomeEntityState = std::derived_from<T, EntityStateContract<T>> &&
	(ImplementsOnEnter<T> || ImplementsOnExit<T> || ImplementsOnUpdate<T>);

/* ENTITY STATE ID */
struct EntityStateFamily;

template <typename T> requires SomeEntityState<T>
struct TypeInFamily<EntityStateFamily, T> : std::true_type {};

using EntityStateID = FamilyTypeID<EntityStateFamily>;
/**/

struct EntityStateTable
{
	using UpdateProcess = void(*)(Entity&, float);
	using TransitionProcess = void(*)(Entity&);

	TransitionProcess onEnter = nullptr;
	TransitionProcess onExit = nullptr;
	UpdateProcess onUpdate = nullptr;
};


template <typename Derived>
inline EntityStateTable EntityStateContract<Derived>::GetStateTable()
{
	EntityStateTable stateTable{};

	if constexpr (ImplementsOnEnter<Derived>)
	{
		stateTable.onEnter = &Derived::OnEnter;
	}
	if constexpr (ImplementsOnExit<Derived>)
	{
		stateTable.onExit = &Derived::OnExit;
	}
	if constexpr (ImplementsOnUpdate<Derived>)
	{
		stateTable.onUpdate = &Derived::OnUpdate;
	}

	return stateTable;
}


class EntityStateSystem : public System
{
public:
	void Update(float delta);

	template <typename T> requires SomeEntityState<T>
	Result<Void> RegisterState()
	{
		const size_t id = EntityStateID::value<T>;
		if (id >= stateTables_.size())
		{
			stateTables_.resize(id + 1);
		}

		if (StateTableRegistered(id))
		{
			return MAKE_ERROR_FMT("EntityState with ID '{}'"
				" already registered with Entity State System", id);
		}

		stateTables_[id] = T::GetStateTable();

		return Void{};
	}

private:
	bool StateTableRegistered(size_t id) const
	{
		return id < stateTables_.size() && (stateTables_[id].onEnter || 
											stateTables_[id].onExit || 
											stateTables_[id].onUpdate);
	}

	std::vector<EntityStateTable> stateTables_;
};