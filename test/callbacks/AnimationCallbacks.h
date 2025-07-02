#pragma once
#include "../../events/data/EventDataIncludes.h"
#include "../../components/ComponentIncludes.h"
#include "../../ecs/Ecs.h"

namespace test {

auto SpriteAdvanceOnDistanceTraveled(float targetDist)
{
    struct
    {
        float targetDistance;
        mutable float delta = 0.0f;

        ReturnSignal operator()(Entity_t id, const events::EntityPositionChanged& ev) const
        {
            auto entity = ECS::GetEntityByID(id);
            if (!entity.IsValid())
            {
                return ReturnSignal::StopObserving;
            }

            SDL_FPoint dist = { ev.newPosition - ev.oldPosition };
            delta += std::sqrt(dist.x * dist.x + dist.y * dist.y);

            if (delta >= targetDistance)
            {
                delta = 0.0f;

                if (!entity.HasComponent<SpriteAnimations>())
                {
                    return ReturnSignal::KeepObserving;
                }

                auto& animations = entity.GetComponent<SpriteAnimations>().map;

                bool didIncrement = animations.NextInSeries();
                assert(didIncrement);
            }

            return ReturnSignal::KeepObserving;
        }
    } callback{ .targetDistance = targetDist };

    return callback;
}




} // test