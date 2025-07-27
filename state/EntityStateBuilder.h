#pragma once
#include "../ecs/Ecs.h"
#include "../scripting/TypedLuaFunction.h"

class EntityStateBuilder
{
public:
	static Result<EntityStateBuilder> GetInstance(Entity entity);

	template <typename Fn>
	EntityStateBuilder& WithEnterTransition(std::string_view transitionName, Fn&& fn)
	{
		onEnterTransition_ = std::make_pair( 
			HashName{ transitionName },
			StateTransitionCallback{ std::forward<Fn>(fn) }
		);

		return *this;
	}
	EntityStateBuilder& WithEnterTransition(std::string_view transitionName,
											TypedLuaFunction<Void(Entity&)> luaFn)
	{
		onEnterTransition_ = std::make_pair(
			HashName{ transitionName },
			StateTransitionCallback{ WrapLuaTransition(std::move(luaFn)) }
		);

		return *this;
	}
	EntityStateBuilder& WithEnterTransition(std::string_view transitionName) 
	{
		onEnterTransition_.first = transitionName;
		onEnterTransition_.second = nullptr;

		return *this;
	}

	template <typename Fn>
	EntityStateBuilder& WithNewExitTransition(std::string_view transitionName, Fn&& fn)
	{
		onExitRequest_.transitionName = transitionName;
		onExitRequest_.transitionCallback = std::forward<Fn>(fn);

		return *this;
	}
	EntityStateBuilder& WithNewExitTransition(std::string_view transitionName,
										   TypedLuaFunction<Void(Entity&)> luaFn)
	{
		return WithNewExitTransition(transitionName, WrapLuaTransition(std::move(luaFn)));
	}
	EntityStateBuilder& WithExitTransition(std::string_view transitionName) // if reusing a registered state
	{
		onExitRequest_.transitionName = transitionName;

		return *this;
	}

	EntityStateBuilder& WithStateLinks(std::initializer_list<std::string_view> links)
	{
		stateLinks_ = { links.begin(), links.end() };

		return *this;
	}

	void Build();

private:
	template <typename Fn>
	static EntityStateBuilder& WithTransitionImpl(std::string_view transitionName, Fn&& fn,
												  std::pair<HashName, StateTransitionCallback>& target)
	{
		target = std::make_pair(
			HashName{ transitionName },
			StateTransitionCallback{ std::forward<Fn>(fn) }
		);

		return *this; 
	}
	EntityStateBuilder& WithTransitionImpl(std::string_view transitionName,
										   TypedLuaFunction<Void(Entity&)> luaFn,
										   std::pair<HashName, StateTransitionCallback>& target)
	{
		onEnterTransition_ = std::make_pair(
			HashName{ transitionName },
			StateTransitionCallback{ WrapLuaTransition(std::move(luaFn)) }
		);

		return *this;
	}

	static StateTransitionCallback WrapLuaTransition(TypedLuaFunction<Void(Entity&)>&& luaFn);

	EntityStateBuilder(Entity entity, std::string_view stateNm) : entity_(entity), 
		stateName_(HashName{ stateNm }), onEnterRequest_{ .stateName = stateName_ }, 
		onExitRequest_{ .stateName = stateName_ } {}

	Entity entity_;
	HashName stateName_;
	std::pair<HashName, StateTransitionCallback> onEnterTransition_;
	std::pair<HashName, StateTransitionCallback> onExitTransition_;
	EntityState::StateLinks stateLinks_;
};