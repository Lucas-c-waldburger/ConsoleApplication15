#pragma once
#include "../ecs/EntityT.h"
#include "../callbacks/EventCallbackRegistry.h"
#include "../events/EventDispatchListenerInterface.h"
#include "../systems/System.h"

class Entity;

class EventCallbackSystem : public System,
							public EventDispatchListener<EventCallbackSystem> 
{
public:
	class CallbackRegistry
	{
	public:
		template <typename T>
		std::pair<uint32_t, EventCallbackView> RegisterCallback(std::string_view callbackName, T&& fnOrLua)
		{
			return impl_.RegisterCallback(callbackName, std::forward<T>(fnOrLua));
		}

		std::pair<uint32_t, EventCallbackView> RegisterCallback(EventCallbackFulfillmentRequest&& request);

		std::pair<uint32_t, EventCallbackView> RegisterOrRetrieveCallback(EventCallbackFulfillmentRequest&& request);

		std::pair<uint32_t, EventCallbackView> GetCallback(uint32_t eventType, std::string_view callbackName);
		std::pair<uint32_t, EventCallbackView> GetCallback(uint32_t eventType, HashName callbackNameHash);

		bool EraseCallback(uint32_t eventType, std::string_view callbackName);

	private:
		EventCallbackRegistry impl_;
	};

	CallbackRegistry& GetCallbackRegistry() { return registry_; }

	void Dispatch(EventSpan events);

private:
	void HandleEventCallbackFulfillmentRequests();
	void HandleControllerInputCallback(Entity& entity, const Event& event);
	void HandleEventCallback(Entity& entity, const Event& event);

	CallbackRegistry registry_;
}; 

