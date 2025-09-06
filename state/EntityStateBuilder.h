//#pragma once
//#include "../ecs/Ecs.h"
//#include "../callbacks/StateTransitionCallbackRegistry.h"
//
//class EntityStateBuilder
//{
//public:
//	explicit EntityStateBuilder(StateTransitionCallbackRegistry& registry) :
//		registry_(registry) {}
//
//	EntityStateBuilder& WithName(std::string_view stateName)
//	{
//		state_.stateName = HashName{ stateName };
//		return *this;
//	}
//
//	template <typename Fn> requires std::convertible_to<Fn, StateTransitionCallback>
//	EntityStateBuilder& WithNewEnterTransition(std::string_view callbackName, Fn&& fn)
//	{
//		state_.transitions.onEnter = registry_.RegisterCallback(callbackName, std::forward<Fn>(fn));
//		return *this;
//	}
//	EntityStateBuilder& WithEnterTransition(std::string_view callbackName)
//	{
//		state_.transitions.onEnter = registry_.GetCallbackView(callbackName);
//		return *this;
//	}
//
//	template <typename Fn> requires std::convertible_to<Fn, StateTransitionCallback>
//	EntityStateBuilder& WithNewExitTransition(std::string_view callbackName, Fn&& fn)
//	{
//		state_.transitions.onExit = registry_.RegisterCallback(callbackName, std::forward<Fn>(fn));
//		return *this;
//	}
//	EntityStateBuilder& WithExitTransition(std::string_view callbackName)
//	{
//		state_.transitions.onExit = registry_.GetCallbackView(callbackName);
//		return *this;
//	}
//
//	EntityStateBuilder& WithStateLinks(std::initializer_list<std::string_view> stateNames)
//	{
//		state_.stateLinks.reserve(stateNames.size());
//		std::transform(stateNames.begin(), stateNames.end(), std::back_inserter(state_.stateLinks),
//			[](auto name) { return HashName{ name }; });
//		return *this;
//	}
//
//	std::pair<HashName, EntityState> Build() 
//	{ 
//		return std::make_pair(stateName_, std::move(state_));
//	}
//
//private:
//	EntityState state_;
//	StateTransitionCallbackRegistry& registry_;
//};