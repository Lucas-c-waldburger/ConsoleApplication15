#pragma once
//#include <unordered_map>
//#include "SpriteAnimationSeries.h"
//#include "../core/commonObjects.h"
//#include "../core/Algorithms.h"
//#include "../deps/function2/function2.hpp"
//#include <typeindex>
//#include "../components/driver/BaseDriver.h"
//#include "../components/EntityStateComponent.h"
//#include "../ecs/Ecs.h";
//#include "../callbacks/BaseCallbackDescriptor.h"
#include "../ecs/Ecs.h"
#include "../physics/B2Shape.h"
#include "../callbacks/EventCallbackRegistry.h"
#include "../callbacks/StateTransitionCallbackRegistry.h"


template <typename Fn>
concept FunctionReturningCallable = 
	HasFuncTraits<Fn> && HasFuncTraits<typename func_traits<Fn>::return_type>;

template <typename Fn, auto* wrapper = nullptr> requires FunctionReturningCallable<Fn>
class CurryableCallback  
{
public:
	template <typename F>
	CurryableCallback(std::string_view nm, F&& fn) :
		name_(std::string{nm}), callback_(std::forward<F>(fn)) {}

	template <typename...Args> requires std::invocable<Fn, Args...>
	auto MakeInstance(Args&&...args)
	{
		if constexpr (wrapper)
		{
			return std::invoke(wrapper, callback_, args...);
		}
		return std::invoke(callback_, args...);
	}

	const std::string& GetName() const { return name_; }

private:	
	std::string name_;
	Fn callback_;
};


//inline Result<Entity> GetRootBodyEntity(const CollisionData& collisionData)
//{
//	auto shapeEntity = ECS::GetEntityByID(collisionData.entity);
//	if (!shapeEntity.IsValid())
//	{
//		return MAKE_ERROR("Shape entity invalid");
//	}
//
//	if (!shapeEntity.HasComponent<Collider>())
//	{
//		return MAKE_ERROR("Shape entity did not have a collider component");
//	}
//	
//	if (shapeEntity.GetComponent<Collider>().shape.GetData().GetHandle() !=
//		collisionData.shapeHandle)
//	{
//		return MAKE_ERROR("Collision data shape handle did not belong to shape entity");
//	}
//
//	if (shapeEntity.HasComponent<RigidBody>())
//	{
//		if (!shapeEntity.GetComponent<RigidBody>().body.GetData()
//			.OwnsShape(collisionData.shapeHandle))
//		{
//			return MAKE_ERROR("Shape entity had a rigid body component that did not "
//				"own the shape from its collider component");
//		}
//
//		return shapeEntity;
//	}
//
//	auto shapeEntityRelations = shapeEntity.GetRelations();
//	if (!shapeEntityRelations.IsChild())
//	{
//		return MAKE_ERROR("Entity had a collider component without a rigid body component "
//			"but was not a child of a parent's rigid body");
//	}
//
//	auto parent = shapeEntityRelations.GetParent();
//
//	if (!parent.IsValid())
//	{
//		return MAKE_ERROR("Parent body entity was invalid");
//	}
//	if (!parent.HasComponent<RigidBody>())
//	{
//		return MAKE_ERROR("Parent body did not have a rigid body component");
//	}
//	if (!parent.GetComponent<RigidBody>().body.GetData()
//		.OwnsShape(collisionData.shapeHandle))
//	{
//		return MAKE_ERROR("Parent body did not own child's shape");
//	}
//
//	return parent;
//}







//
//inline bool EntityStatesContainTransition(EntityStates& states, std::string_view transitionName)
//{
//	return std::any_of(states.table.begin(), states.table.end(), [transitionName](const auto& pair) {
//		return pair.second.transitions.onEnter.name == transitionName ||
//			   pair.second.transitions.onExit.name == transitionName;
//	}); 
//}

//struct StateTransitionCallbackDescriptor : BaseCallbackDescriptor
//{
//	bool operator==(const StateTransitionCallbackDescriptor& rhs) const
//	{
//		return static_cast<const BaseCallbackDescriptor&>(*this) == 
//			   static_cast<const BaseCallbackDescriptor&>(rhs);
//	}
//};
//
//namespace std {
//	template <>
//	struct hash<StateTransitionCallbackDescriptor> {
//		size_t operator()(const StateTransitionCallbackDescriptor& desc) const noexcept {
//			std::hash<BaseCallbackDescriptor>{}(static_cast<const BaseCallbackDescriptor&>(desc));
//		}
//	};
//}
//
//using StateTransitionCallbackRegistryTable = CallbackRegistryTable<StateTransitionCallbackDescriptor>;
//using StateTransitionCallbackFn = StateTransitionCallbackRegistryTable::CallbackFn;
//using StateTransitionCallbackFnView = StateTransitionCallbackRegistryTable::CallbackFnView;
//
//class StateTransitionCallbackRegistry
//{
//public:
//	struct RegistrationOutcome
//	{
//		Handle<StateTransitionCallbackDescriptor> handle = {};
//		bool newlyRegistered = false;
//	};
//
//	StateTransitionCallbackRegistry() = default;
//	~StateTransitionCallbackRegistry() = default;
//
//	StateTransitionCallbackRegistry(const StateTransitionCallbackRegistry&) = delete;
//	StateTransitionCallbackRegistry& operator=(const StateTransitionCallbackRegistry&) = delete;
//
//	StateTransitionCallbackRegistry(StateTransitionCallbackRegistry&& rhs) noexcept :
//		masterTable_(std::move(rhs.masterTable_)) {}
//	StateTransitionCallbackRegistry& operator=(StateTransitionCallbackRegistry&& other) noexcept
//	{
//		if (this != &other)
//		{
//			masterTable_ = std::move(other.masterTable_);
//		}
//		return *this;
//	}
//
//
//
//
//private:
//	StateTransitionCallbackRegistryTable masterTable_;
//};

//class EntityStateManager
//{
//public:
//
//private:
//};

//inline bool NeedsTransitionUpdate()

//using EntityStateTable = std::unordered_map<
//
//
//class EntityStateTables
//{
//public:
//
//private:
//};




//class EntityStateDriver : public BaseDriver<EntityStateDriver, EntityStates>
//{
//public:
//	friend class Super;
//
//	struct TransitionNamePair
//	{
//		std::string_view onEnterName;
//		std::string_view onExitName;
//	};
//
//	Result<Void> AddState(std::string_view stateName, TransitionNamePair transitionNames, 
//						  std::initializer_list<std::string_view> links)
//	{
//		auto& states = GetComponent<EntityStates>();
//		HashName stateNameHash{ stateName };
//
//		if (states.table.contains(stateNameHash))
//		{
//			return MAKE_ERROR_FMT("duplicate state name '{}' in entity's state table", stateName);
//		}
//
//		auto& newState = states.table[stateNameHash];
//		
//		newState.transitions.onEnter.name = transitionNames.onEnterName;
//		newState.transitions.onExit.name = transitionNames.onExitName;
//		newState.stateLinks = { links.begin(), links.end() };
//
//		if (states.table.size() == 1)
//		{
//			states.current = stateNameHash;
//		}
//
//		auto& update = GetEntity().AddComponent<NeedsUpdate>();
//
//		update.components |= EntityStates::componentBit;
//
//		return Void{};
//	}
//
//	bool EraseState(std::string_view stateName)
//	{
//		auto& states = GetComponent<EntityStates>();
//		HashName stateNameHash{ stateName };
//
//		bool erased = states.table.erase(stateNameHash);
//		if (!erased)
//		{
//			return false;
//		}
//
//		for (auto& [_, states] : states.table)
//		{
//			states.stateLinks.erase(stateNameHash);
//		}
//
//		if (states.current == stateNameHash)
//		{
//			states.current = kInvalidHashName;
//		}
//
//		return true;
//	}
//
//	Result<Void> ChangeState(std::string_view nextStateName)
//	{
//		auto& states = GetComponent<EntityStates>();
//		HashName nextStateNameHash{ nextStateName };
//
//		auto it = states.table.find(nextStateNameHash);
//		if (it == states.table.end())
//		{
//			return MAKE_ERROR_FMT("State name '{}' not found in entity's state table", nextStateName);
//		}
//
//		if (auto* currentState = GetCurrentEntityState(states))
//		{
//			if (!currentState->stateLinks.contains(nextStateNameHash))
//			{
//				return MAKE_ERROR_FMT("State name '{}' not linked to entity's current state: '{}'",
//					nextStateName, states.current);
//			}
//
//			if (currentState->transitions.onExit.fn)
//			{
//				currentState->transitions.onExit(GetEntity());
//			}
//		}
//
//		states.current = nextStateNameHash;
//
//		auto& newState = states.table[nextStateNameHash];
//
//		if (newState.transitions.onEnter.fn)
//		{
//			newState.transitions.onEnter(GetEntity());
//		}
//
//		return Void{};
//	}
//
//private:
//	explicit EntityStateDriver(Entity ent) : BaseDriver(ent) {}
//};


//using EntityStateMap = std::unordered_map<HashName, 

//class EntityStateMap
//{
//public:
//
//private:
//};


//// TODO: think about moving the dirty state logic into a tag component
//class SpriteAnimationsTable
//{
//public:
//    enum ResetOption : uint8_t
//    {
//        None = 0,
//        Index = 1 << 0,
//        SpriteRange = 1 << 1,
//        All = 0xFF
//    };
//
//    using MapType = std::unordered_map<std::string, SpriteAnimationSeries,
//                                       TransparentStringHash, std::equal_to<>>;
//    using EmplaceRetType = std::pair<typename MapType::iterator, bool>;
//
//    // map operations
//    EmplaceRetType Emplace(std::string_view seriesName, const SpriteAnimationSeries& series);
//    EmplaceRetType Emplace(std::string_view seriesName, SpriteAnimationSeries&& series);
//
//    bool Erase(std::string_view seriesName);
//
//    bool Contains(std::string_view seriesName) const;
//
//    // current series operations
//    const SpriteAnimationSeries* GetCurrentSeries() const;
//
//    bool HasCurrentSeries() const;
//
//    // by default, setting a new current will reset index to 0 and set sprite range to the full series range
//    bool SetCurrentSeries(std::string_view seriesName, ResetOption resetOptions);
//    bool SetCurrentSeries(std::string_view seriesName);
//
//    bool SetCurrentIndex(size_t newIndex);
//
//    // define a sub-index range to be advanced through instead of the full sequence
//    bool SetCurrentSpriteRange(Range<size_t> newRange, bool bumpIndexNow);
//    bool SetCurrentSpriteRange(Range<size_t> newRange);
//    void ResetCurrentSpriteRange();
//
//    // increment current series sprite. true means we have a new sprite to render
//    bool NextInSeries();
//
//    // update operations
//    bool NeedsUpdate() const { return dirty_; }
//    void MarkUpdated() { dirty_ = false; }
//
//private:
//    MapType map_;
//    std::string_view current_;
//    bool dirty_ = true;
//};
