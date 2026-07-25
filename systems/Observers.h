#pragma once
#include "../core/Signal.h"
#include "../ecs/Ecs.h"

template <typename Derived>
class EntityDestroyedObserver
{
protected:
	void ObserveEntityDestroyed()
	{
		signalToken_ = ECS::ObserveEntityDestroyed([this](Entity e) {
			static_cast<Derived*>(this)->OnEntityDestroyed(e);
			});
	}

private:
	SignalToken signalToken_;
};

template <typename Derived>
class ComponentRemovedObserver
{

protected:
	void ObserveComponentRemoved()
	{
		signalToken_ = ECS::ObserveComponentRemoved([this](Entity e, ComponentSignature sig) {
			static_cast<Derived*>(this)->OnComponentRemoved(e, sig);
			});
	}

private:
	SignalToken signalToken_;
};