#pragma once
#include <optional>
#include "../IEventData.h"
#include "../../ecs/EntityT.h"
#include "EntityEventConcept.h"
#include <SDL_rect.h>

namespace events {

struct EntityCreated : IEventData<EntityCreated>
{
	Entity_t entity = kInvalidEntity;
	std::optional<Entity_t> parent;
};

struct EntityDestroyed : IEventData<EntityDestroyed>
{
	Entity_t entity = kInvalidEntity;
	std::optional<Entity_t> parent;
};

struct EntityPositionChanged : IEventData<EntityPositionChanged>, 
							   EntityParticipants<1>
{
	SDL_FPoint newPosition = { 0.0f, 0.0f };
	SDL_FPoint oldPosition = { 0.0f, 0.0f };
};

} // events
