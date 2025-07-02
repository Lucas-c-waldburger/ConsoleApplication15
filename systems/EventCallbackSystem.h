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
	void EntityDestroyed(Entity_t entityId);

	void Dispatch(EventSpan events);

	EventCallbackRegistry& GetRegistry() { return callbackRegistry_; }

private:
	void HandleControllerInputCallback(Entity& entity, const Event& event);
	void HandleEventCallback(Entity& entity, const Event& event);


	EventCallbackRegistry callbackRegistry_;
}; 

