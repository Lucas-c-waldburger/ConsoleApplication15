#pragma once
#include "../../events/data/EventDataIncludes.h"
#include "../../components/ComponentIncludes.h"
#include "../../ecs/Ecs.h"
#include "../../components/driver/SpriteAnimationDriver.h"

namespace test {

auto SpriteAdvanceOnDistanceTraveled(float targetDist)
{
    struct
    {
        float targetDistance;
        mutable float delta = 0.0f;

        ReturnSignal operator()(Entity& entity, const events::EntityPositionChanged& ev) const
        {
            SDL_FPoint dist = { ev.newPosition - ev.oldPosition };
            delta += std::sqrt(dist.x * dist.x + dist.y * dist.y);

            if (delta >= targetDistance)
            {
                delta = 0.0f;

                auto driver = SpriteAnimationDriver::GetInstance(entity);
                if (!driver.Success())
                {
                    LOG_ERROR(driver.GetError());

                    return ReturnSignal::KeepObserving;
                }

                driver.GetValue().Step();
            }

            return ReturnSignal::KeepObserving;
        }
    } callback{ .targetDistance = targetDist };

    return callback;
}

auto FlipSpriteOnAxisDirection(Direction nativeDirection)
{
    return [nativeDirection](Entity& entity, const events::GameControllerInput& ev)
    {
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