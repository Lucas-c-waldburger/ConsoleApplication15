#pragma once
#include "../../events/data/EventDataIncludes.h"
#include "../../components/ComponentIncludes.h"
#include "../../ecs/Ecs.h"
#include "../Resources.h"
#include "../../inputs/controller/GameController.h"

namespace test {

//template <SomeComponent...Ts>
//inline bool Entity

//constexpr Direction GetControllerAxisDirection(SDL_Point p)
//{
//    using enum Direction;
//
//    Direction dirX = (p.x < 0) ? W : (p.x > 0) ? E : None;
//    Direction dirY = (p.y < 0) ? N : (p.y > 0) ? S : None;
//
//    return static_cast<Direction>((static_cast<uint8_t>(dirX) | static_cast<uint8_t>(dirY)));
//}

//auto ConnectToFirstController()
//{
//    return [](Entity& entity, const events::GameControllerConnected& ev) -> ReturnSignal 
//    {
//        if (!entity.HasComponent<GameControllerState>())
//        {
//            LOG_WARNING("Entity did not have GameControllerState component");
//            return ReturnSignal::StopObserving;
//        }
//
//        auto& controllerState = entity.GetComponent<GameControllerState>();
//        if (controllerState.joystickID == GameController::kInvalidJoystickID)
//        {
//            controllerState.joystickID = ev.joystickID;
//            LOG_INFO("Entity attached to new controller connection!");
//        }
//
//        return ReturnSignal::KeepObserving;
//    };
//}

inline auto ConnectToFirstController(Entity& entity)
{
    return [entity](const events::GameControllerConnected& ev) mutable
    {        
        if (!entity.IsValid())
        {
            LOG_WARNING("Entity was invalid");
            return;
        }

        if (!entity.HasComponent<GameControllerState>())
        {
            LOG_WARNING("Entity did not have GameControllerState component");
            return;
        }

        auto& controllerState = entity.GetComponent<GameControllerState>();
        if (controllerState.joystickID == GameController::kInvalidJoystickID)
        {
            controllerState.joystickID = ev.joystickID;
            LOG_INFO("Entity attached to new controller connection!");
        }
    };
}

//auto DisconnectController()
//{
//    return [](Entity& entity, const events::GameControllerDisconnected& ev) -> ReturnSignal 
//    {
//        if (!entity.HasComponent<GameControllerState>())
//        {
//            LOG_WARNING("Entity did not have GameControllerState component");
//            return ReturnSignal::StopObserving;
//        }
//
//        auto& controllerState = entity.GetComponent<GameControllerState>();
//        if (controllerState.joystickID != ev.joystickID)
//        {
//            LOG_DEBUG("Entity's connected controller different from the one that was disconnected");
//            return ReturnSignal::KeepObserving;
//        }
//
//        controllerState.joystickID = GameController::kInvalidJoystickID;
//
//        LOG_DEBUG("Controller Disconnected. Listening for a new connection on this entity...");
//
//        return ReturnSignal::KeepObserving;
//    };
//}

inline auto DisconnectController(Entity& entity)
{
    return [entity](const events::GameControllerDisconnected& ev) mutable
    {
        if (!(entity.IsValid() && entity.HasComponent<GameControllerState>()))
        {
            return;
        }

        auto& controllerState = entity.GetComponent<GameControllerState>();
        if (controllerState.joystickID == ev.joystickID)
        {
            controllerState.joystickID = GameController::kInvalidJoystickID;

            LOG_DEBUG("Controller Disconnected. Listening for a new connection "
                "on this entity...");
        }
    };
}

//auto ApplyAxisInputToForce(float impulseScale)
//{
//    return [impulseScale](Entity& entity, const events::GameControllerInput& ev) -> ReturnSignal
//    {
//        int axisValueX = ev.input.value.axis.x;     
//        if (axisValueX == 0)
//        {
//            return ReturnSignal::KeepObserving;
//        }
//
//        if (!entity.HasComponent<GameControllerState>() ||
//            entity.GetComponent<GameControllerState>().joystickID != ev.joystickID)
//        {
//            return ReturnSignal::KeepObserving;
//        }
//
//        if (!entity.HasComponent<RigidBody>())
//        {
//            return ReturnSignal::KeepObserving;
//        }
//
//        auto& rigidBody = entity.GetComponent<RigidBody>();
//
//        float impulse = static_cast<float>(axisValueX) * impulseScale;
//
//        rigidBody.forceRequests.impulses.push_back(Force{ .value = { impulse, 0.0f } });
//
//        return ReturnSignal::KeepObserving;
//    };
//}


inline auto ApplyAxisInputToForce(Entity& entity)
{
    return [entity](const events::GameControllerInput& ev) mutable
    {
        if (!entity.IsValid())
        {
            return;
        }

        if (!entity.HasComponents<GameControllerState, RigidBody>())
        {
            return;
        }

        if (entity.GetComponent<GameControllerState>().joystickID != ev.joystickID)
        {
            return;
        }

        auto& rigid = entity.GetComponent<RigidBody>();

        auto [axisValueX, axisValueY] = ev.input.value.axis;

        const SDL_FPoint impulseScale = {
            rigid.limits.maxImpulse.x / static_cast<float>(GameController::kAxisMax),
            rigid.limits.maxImpulse.y / static_cast<float>(GameController::kAxisMax)
        };

        const SDL_FPoint impulse = {
            static_cast<float>(axisValueX) * impulseScale.x,
            static_cast<float>(axisValueY) * impulseScale.y
        };

        rigid.forceRequests.impulses.push_back(Force{ .value = impulse });
    };
}

} // test