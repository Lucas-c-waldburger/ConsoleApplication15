#pragma once
#include "../ecs/EntityT.h"
#include "../events/EventCallbackRegistry.h"
#include "../events/EventDispatchListenerInterface.h"
#include "../systems/System.h"

class EventCallbackSystem : public System,
							public EventDispatchListener<EventCallbackSystem> 
{
public:
	void EntityDestroyed(Entity_t entityId);

	void Dispatch(EventSpan events);

	EventCallbackRegistry& GetRegistry() { return callbackRegistry_; }

private:
	EventCallbackRegistry callbackRegistry_;
}; 

