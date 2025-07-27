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
		template <typename Name, typename T> 
			requires (std::same_as<Name, std::string_view> || std::same_as<Name, HashName>)
		StateTransitionView RegisterTransition(Name transitionName, T&& fnOrLua)
		{
			return impl_.RegisterTransition(transitionName, std::forward<T>(fnOrLua));
		}

		StateTransitionView RegisterTransition(StateTransitionFulfillmentRequest&& request)
		{
			return impl_.RegisterTransition(std::move(request));
		}

		StateTransitionView RegisterOrRetrieveTransition(StateTransitionFulfillmentRequest&& request)
		{
			return impl_.RegisterOrRetrieveTransition(std::move(request));
		}

		template <typename Name> 
			requires (std::same_as<Name, std::string_view> || std::same_as<Name, HashName>)
		StateTransitionView GetTransition(Name transitionName)
		{
			return impl_.GetTransition(transitionName);
		}

		bool EraseTransition(std::string_view transitionName);

	private:
		StateTransitionRegistry impl_;
	};

	TransitionRegistry& GetTransitionRegistry() { return registry_; }

	void Update();

private:
	void HandleTransitionFulfillmentRequest(Entity& entity);

	TransitionRegistry registry_;
};