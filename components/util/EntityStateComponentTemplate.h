#pragma once
#include <concepts>
#include <typeindex>
#include <optional>
#include "../../callbacks/CallbackRegistryTable.h"
#include "../../deps/function2/function2.hpp"
#include "../../core/commonObjects.h"
#include "../../ecs/EntityT.h"

namespace state {

enum EntityState : uint64_t 
{
	Invalid,
	Walking,
	Jumping,
	LeftFace,
	RightFace
};

//struct EntityStateNode;

struct ConditionCallbackDescriptor : BaseCallbackDescriptor {};
struct TransitionCallbackDescriptor : BaseCallbackDescriptor {};

using ConditionCallbackFn = fu2::unique_function<bool(Entity_t)>;
using OnTransitionCallbackFn = fu2::unique_function<void(Entity_t)>;

struct Transition
{
	struct {
		ConditionCallbackDescriptor condition;
		TransitionCallbackDescriptor onTransition;
	} keys;
	int priority = 0;
};

struct StateNode
{
	uint64_t entityState = 0;
	std::vector<Transition> transitions;
};

class StateTree
{
public:

private:
};

}

class StateCallbackRegistry
{
public:

private:

};

//template <typename StateEnum>
//concept EntityStateEnum = std::is_enum_v<StateEnum> &&
//std::same_as<std::underlying_type_t<StateEnum>, uint8_t> ;
//
//template <typename StateEnum> requires  std::is_enum_v<StateEnum>
//struct IEntityStateComponent
//{
//	StateEnum state;
//	float stateDuration = 0.0f;
//};
//
//struct EntityState
//{
//	uint64_t bit = 0;
//};