#pragma once
#include "System.h"
#include "../callbacks/StateTransitionCallbackRegistry.h"


class EntityStateSystem : public System
{
public:
	// extends EraseTransition to remove dead transitions from entity states
	class TransitionRegistry
	{
	public:
		template <typename T>
		StateTransitionView RegisterTransition(std::string_view transitionName, T&& fnOrLua)
		{
			return impl_.RegisterTransition(transitionName, std::forward<T>(fnOrLua));
		}

		StateTransitionView GetTransition(std::string_view transitionName);
		StateTransitionView GetTransition(HashName transitionHashName);

		bool EraseTransition(std::string_view transitionName);

	private:
		StateTransitionRegistry impl_;
	};

	TransitionRegistry& GetTransitionRegistry() { return registry_; }

	void Update();

private:
	void FulfillTransitionRequest(StateTransitionView& transition);

	TransitionRegistry registry_;
};