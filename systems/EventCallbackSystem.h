//#pragma once
//#include "../ecs/EntityT.h"
//#include "../callbacks/EventCallbackRegistry.h"
//#include "../events/EventDispatchListenerInterface.h"
//#include "../systems/System.h"
//
//class Entity;
//
//class EventCallbackSystem : public System,
//							public EventDispatchListener<EventCallbackSystem> 
//{
//public:
//	EventCallbackRegistry& GetEventCallbackRegistry() { return registry_; }
//
//	void Dispatch(EventSpan events);
//
//private:
//	void HandleControllerInputCallback(Entity& entity, const Event& event);
//	void HandleEventCallback(Entity& entity, const Event& event);
//
//	EventCallbackRegistry registry_;
//}; 
//
