#pragma once
#include "StateTransitionCallback.h"
#include "../scripting/TypedLuaFunction.h"
#include "../ecs/Ecs.h"

class StateTransitionRegistry
{
public:
	StateTransitionRegistry() = default;
	~StateTransitionRegistry() = default;

	StateTransitionRegistry(const StateTransitionRegistry&) = delete;
	StateTransitionRegistry& operator=(const StateTransitionRegistry&) = delete;

	StateTransitionRegistry(StateTransitionRegistry&& rhs) noexcept :
		masterTable_(std::move(rhs.masterTable_)) {}
	StateTransitionRegistry& operator=(StateTransitionRegistry&& other) noexcept
	{
		if (this != &other)
		{
			masterTable_ = std::move(other.masterTable_);
		}
		return *this;
	}

	template <typename Fn>
	StateTransitionView RegisterTransition(std::string_view transitionName, Fn&& fn)
	{
		HashName hashTransitionName{ transitionName };

		if (masterTable_.contains(hashTransitionName))
		{
			return {};
		}

		auto [it, inserted] = masterTable_.emplace(hashTransitionName, std::forward<Fn>(fn));
		assert(inserted);

		return StateTransitionView{
			.name = hashTransitionName,
			.fn = it->second
		};
	}

	StateTransitionView RegisterTransition(std::string_view transitionName,
										   TypedLuaFunction<Void(Entity&)> luaFn)
	{
		HashName hashTransitionName{ transitionName };

		if (masterTable_.contains(hashTransitionName))
		{
			return {};
		}

		auto wrapped = [fn = std::move(luaFn)](Entity& entity) -> void { 
			auto result = fn(entity);
			if (!result.Success())
			{
				LOG_ERROR(result.GetError());
			}
		};

		auto [it, inserted] = masterTable_.emplace(hashTransitionName, std::move(wrapped));
		assert(inserted);

		return StateTransitionView{
			.name = hashTransitionName,
			.fn = it->second
		};
	}

	bool EraseTransition(std::string_view transitionName)
	{
		HashName transitionNameHash{ transitionName };

		auto it = masterTable_.find(transitionNameHash);
		if (it == masterTable_.end())
		{
			return false;
		}

		masterTable_.erase(it);

		return true;
	}

	StateTransitionView GetTransition(HashName transitionHashName)
	{
		auto it = masterTable_.find(transitionHashName);
		if (it == masterTable_.end())
		{
			return {};
		}

		return StateTransitionView{
			.name = transitionHashName,
			.fn = it->second
		};
	}

	StateTransitionView GetTransition(std::string_view transitionName)
	{
		return GetTransition(HashName{ transitionName });
	}

private:
	HashNameMap<StateTransitionCallback> masterTable_;
};