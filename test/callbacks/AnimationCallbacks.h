#pragma once
#include "../../events/data/EventDataIncludes.h"
#include "../../components/ComponentIncludes.h"
#include "../../ecs/Ecs.h"
#include "../../core/CommonEntityMethods.h"
#include "../../components/driver/SpriteAnimationDriver.h"
#include "../Resources.h"

namespace test {

static constexpr std::string_view kLastTransformProxyChildName = "last_transform_proxy";

auto ResetWalkSpriteIfNoMovementAndUpdate()
{
    return [](Entity& entity, const events::GameLoopStepRender& ev) 
    {
        auto tfProxy = entity.GetRelations().FindChild(kLastTransformProxyChildName);
        if (!tfProxy.IsValid())
        {
            LOG_ERROR("last frame transform proxy child was invalid");
            return ReturnSignal::KeepObserving;
        }
        if (!tfProxy.HasComponent<Transform>())
        {
            LOG_ERROR("last frame transform proxy child did not have transform component");
            return ReturnSignal::KeepObserving;
        }

        auto& lastTransform = tfProxy.GetComponent<Transform>();
        
        if (!entity.HasComponent<Transform>())
        {
            LOG_ERROR("entity did not have transform component");
            return ReturnSignal::KeepObserving;
        }

        const auto& currentTransform = entity.GetComponent<Transform>();

        if (lastTransform.position == currentTransform.position) // no movement since last frame
        {
            auto animDriver = SpriteAnimationDriver::GetInstance(entity);
            if (!animDriver.Success())
            {
                LOG_ERROR(animDriver.GetError());
                return ReturnSignal::KeepObserving;
            }

            auto currentSeriesName = animDriver->GetCurrentSeriesName();
            if (currentSeriesName != kWalkSeriesName)
            {
                LOG_ERROR_FMT("expected sprite series '{}', got '{}'", 
                    kWalkSeriesName, currentSeriesName);
                return ReturnSignal::KeepObserving;
            }

            animDriver->SetCurrentSeriesIndex(0);
        }

        lastTransform = currentTransform;

        return ReturnSignal::KeepObserving;
    };
}

auto SpriteAdvanceOnDistanceTraveled(Entity& entity, float targetDist)
{
    struct
    {
        float targetDistance;
        Entity_t id;
        mutable float delta = 0.0f;

        void operator()(const events::EntityPositionChanged& ev) const
        {
            auto entity = ECS::GetEntityByID(id);
            assert(entity.IsValid());

            SDL_FPoint dist = { ev.newPosition - ev.oldPosition };
            delta += std::sqrt(dist.x * dist.x + dist.y * dist.y);

            if (delta >= targetDistance)
            {
                delta = 0.0f;

                auto driver = SpriteAnimationDriver::GetInstance(entity);
                if (!driver.Success())
                {
                    LOG_ERROR(driver.GetError());

                    return;
                }

                driver.GetValue().AdvanceCurrentSeries();
            }
        }
    } callback{ .targetDistance = targetDist, .id = entity.GetID() };

    return callback;
}

auto FlipSpriteOnAxisDirection(Direction nativeDirection)
{
    return [nativeDirection](Entity& entity, const events::GameControllerInput& ev)
    {
        if (!entity.HasComponent<Renderable>())
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

        auto& renderable = entity.GetComponent<Renderable>();
        renderable.profile.flip = newFlip;

        return ReturnSignal::KeepObserving;
    };
}

auto LockLastSeriesIndex()
{
    return [](Entity& entity, const events::SpriteIndexChange& ev) -> ReturnSignal 
    {
        if (ev.entity != entity.GetID())
        {
            return ReturnSignal::KeepObserving;
        }

        auto driver = SpriteAnimationDriver::GetInstance(entity);
        if (!driver.Success())
        {
            LOG_ERROR(driver.GetError());
            return ReturnSignal::KeepObserving;
        }

        size_t seriesSize = driver->GetCurrentSeriesSize();
        if (seriesSize == 0)
        {
            return ReturnSignal::KeepObserving;
        }

        size_t backIndex = seriesSize - 1;

        if (ev.index.now != backIndex)
        {
            return ReturnSignal::KeepObserving;
        }

        driver->SetCurrentSeriesRange({ backIndex, backIndex });
        driver->DisableCurrentSeriesEvents<events::SpriteIndexChange>();

        return ReturnSignal::StopObserving;
    };
}

auto TriggerJumpStateDescend()
{
    return [](Entity& entity, events::EntityPositionChanged& ev) -> ReturnSignal
    {
        if (ev.entity != entity.GetID())
        {
            return ReturnSignal::KeepObserving;
        }

        assert(entity.HasComponent<RigidBody>());
        const auto& body = entity.GetComponent<RigidBody>().body.GetData();

        SDL_FPoint velocity = body.GetLinearVelocity();

        return ReturnSignal::KeepObserving;
    };
}



//auto AdvanceJumpRiseAnimation()
//{
//    return [](Entity& entity, events::SpriteIndexChange& ev) -> ReturnSignal
//    {
//        if (ev.entity != entity.GetID())
//        {
//            return ReturnSignal::KeepObserving;
//        }
//
//        auto animationDriver = SpriteAnimationDriver::GetInstance(entity);
//        if (!animationDriver.Success())
//        {
//            LOG_ERROR(animationDriver.GetError());
//            return ReturnSignal::KeepObserving;
//        }
//
//        size_t rangeMax = animationDriver->GetCurrentSeriesRange().max;
//        //size_t spritesTillFall = (seriesSize - 1) - ev.index.now;
//
//        return ReturnSignal::KeepObserving;
//    };
//}


} // test