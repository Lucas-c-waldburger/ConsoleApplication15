#pragma once
#include "../core/CatTypeID.h"

class Entity;

/* ENTITY STATE TABLE */
struct EntityStateTable
{
	using UpdateProcess = void(*)(Entity&, float);
	using TransitionProcess = void(*)(Entity&);

	TransitionProcess onEnter = nullptr;
	TransitionProcess onExit = nullptr;
	UpdateProcess onUpdate = nullptr;
};
/**/


/* ENTITY STATE IMPLEMENTS */
template <typename T>
concept ImplementsOnEnter = requires {
	{ &T::OnEnter } -> std::convertible_to<EntityStateTable::TransitionProcess>;
};
template <typename T>
concept ImplementsOnExit = requires {
	{ &T::OnExit } -> std::convertible_to<EntityStateTable::TransitionProcess>;
};
template <typename T>
concept ImplementsOnUpdate = requires {
	{ &T::OnUpdate } -> std::convertible_to<EntityStateTable::UpdateProcess>;
};
//template <typename T>
//concept ImplementsOnEnter = 
//	std::same_as<decltype(T::OnEnter), EntityStateTable::TransitionProcess>;
//template <typename T>
//concept ImplementsOnExit = 
//	std::same_as<typename T::OnExit, EntityStateTable::TransitionProcess>;
//template <typename T>
//concept ImplementsOnUpdate = 
//	std::same_as<typename T::OnUpdate, EntityStateTable::UpdateProcess>;
/**/

/* ENTITY STATE CONCEPT */
// FWD DECL
template <typename T>
class EntityState;

template <typename T>
concept SomeEntityState = requires() {
	(ImplementsOnEnter<T> || ImplementsOnExit<T> || ImplementsOnUpdate<T>);
	{ EntityState<T>::GetStateTable() } -> std::same_as<EntityStateTable>;
	{ EntityState<T>::GetStateID() } -> std::same_as<size_t>;
};
/**/


/* ENTITY STATE FAMILY/ID */
struct EntityStateFamily;

template <SomeEntityState T>
struct TypeInFamily<EntityStateFamily, T> : std::true_type {};

using EntityStateID = FamilyTypeID<EntityStateFamily>;
/**/


/* ENTITY STATE WRAPPER */
template <typename State>
class EntityState
{
public:
	static EntityStateTable GetStateTable()
	{
		EntityStateTable stateTable{};

		if constexpr (ImplementsOnEnter<State>)
		{
			stateTable.onEnter = &State::OnEnter;
		}
		if constexpr (ImplementsOnExit<State>)
		{
			stateTable.onExit = &State::OnExit;
		}
		if constexpr (ImplementsOnUpdate<State>)
		{
			stateTable.onUpdate = &State::OnUpdate;
		}

		return stateTable;
	}

	static constexpr size_t GetStateID()
	{
		return EntityStateID::value<State>;
	}
};
/**/




