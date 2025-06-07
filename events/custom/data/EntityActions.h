#pragma once
#include <optional>
#include "../CustomEventData.h"
#include "../../../ecs/EntityT.h"

namespace events {

struct EntityCreated : CustomEvent<EntityCreated>
{
	Entity_t entity = kInvalidEntity;
	std::optional<Entity_t> parent;
};

struct EntityDestroyed : CustomEvent<EntityDestroyed>
{
	Entity_t entity = kInvalidEntity;
	std::optional<Entity_t> parent;
};

struct EntityPositionChanged : CustomEvent<EntityPositionChanged>
{
	Entity_t entity = kInvalidEntity;
	SDL_FPoint newPosition = { 0.0f, 0.0f };
	SDL_FPoint oldPosition = { 0.0f, 0.0f };
};

} // events
