#pragma once
#include "../../events/data/EventDataIncludes.h"
#include "../../components/ComponentIncludes.h"
#include "../../components/driver/SpriteAnimationDriver.h"
#include "../../core/Direction.h"

namespace test {

auto SpriteAdvanceOnDistanceTraveled(float targetDistance)
{
    return [targetDistance, delta = 0.0f](Entity_t id, const events::EntityPositionChanged& ev) mutable
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

            auto driver = SpriteAnimationDriver::GetInstance(entity);
            if (!driver.Success())
            {
                return ReturnSignal::KeepObserving;
            }

            driver->Step();
        }

        return ReturnSignal::KeepObserving;
    };
}

auto FlipSpriteOnAxisDirection(Direction nativeDirection)
{
    return [nativeDirection]
    (Entity_t id, const events::GameControllerInput& ev)
    {
        auto entity = ECS::GetEntityByID(id);
        if (!entity.IsValid())
        {
            return ReturnSignal::StopObserving;
        }
        if (!entity.HasComponent<NewRenderable>())
        {
            return ReturnSignal::KeepObserving;
        }
        if (!entity.HasComponent<GameControllerState>() ||
             entity.GetComponent<GameControllerState>().joystickID != ev.joystickID)
        {
            return ReturnSignal::KeepObserving;
        }

        assert(ev.input.source == GameControllerInputSource::LeftStickAxis ||
               ev.input.source == GameControllerInputSource::RightStickAxis);

        auto newDirection = GetDirectionFromPoint(ev.input.value.axis);
        
        SDL_RendererFlip newFlip = (ShouldFlipHorizontally(nativeDirection, newDirection))
            ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

        auto& renderable = entity.GetComponent<NewRenderable>();
        renderable.profile.flip = newFlip;

        return ReturnSignal::KeepObserving;
    };
}


} // test