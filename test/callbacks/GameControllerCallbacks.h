#pragma once
#include "../../events/data/EventDataIncludes.h"
#include "../../components/ComponentIncludes.h"
#include "../../ecs/Ecs.h"

namespace test {

auto ConnectToFirstController()
{
    return [](Entity_t id, const events::GameControllerConnected& ev) -> ReturnSignal {
        auto ent = ECS::GetEntityByID(id);

        if (!ent.IsValid())
        {
            LOG_WARNING("Entity was invalid, can't connect controller");
            return ReturnSignal::StopObserving;
        }
        if (!ent.HasComponent<GameControllerState>())
        {
            LOG_WARNING("Entity did not have GameControllerState component");
            return ReturnSignal::StopObserving;
        }

        auto& controllerState = ent.GetComponent<GameControllerState>();
        if (controllerState.joystickID == GameController::kInvalidJoystickID)
        {
            controllerState.joystickID = ev.joystickID;
            LOG_INFO("Entity attached to new controller connection!");
        }

        return ReturnSignal::KeepObserving;
    };
}

auto DisconnectController()
{
    return [](Entity_t id, const events::GameControllerDisconnected& ev) -> ReturnSignal {
        auto ent = ECS::GetEntityByID(id);

        if (!ent.IsValid())
        {
            LOG_WARNING("Entity was invalid");
            return ReturnSignal::StopObserving;
        }
        if (!ent.HasComponent<GameControllerState>())
        {
            LOG_WARNING("Entity did not have GameControllerState component");
            return ReturnSignal::StopObserving;
        }

        auto& controllerState = ent.GetComponent<GameControllerState>();
        if (controllerState.joystickID != ev.joystickID)
        {
            LOG_DEBUG("Entity's connected controller different from the one that was disconnected");
            return ReturnSignal::KeepObserving;
        }

        controllerState.joystickID = GameController::kInvalidJoystickID;

        LOG_DEBUG("Controller Disconnected. Listening for a new connection on this entity...");

        return ReturnSignal::KeepObserving;
    };
}

auto ApplyAxisInputToForce(float impulseScale)
{
    return [impulseScale](Entity_t id, const events::GameControllerInput& ev) -> ReturnSignal
    {
        int axisValueX = ev.input.value.axis.x;
        if (axisValueX == 0)
        {
            return ReturnSignal::KeepObserving;
        }

        auto entity = ECS::GetEntityByID(id);
        if (!entity.IsValid())
        {
            return ReturnSignal::StopObserving;
        }

        if (!entity.HasComponent<GameControllerState>() ||
            entity.GetComponent<GameControllerState>().joystickID != ev.joystickID)
        {
            return ReturnSignal::KeepObserving;
        }

        if (!entity.HasComponent<RigidBody>())
        {
            return ReturnSignal::KeepObserving;
        }

        auto& rigidBody = entity.GetComponent<RigidBody>();

        float impulse = static_cast<float>(axisValueX) * impulseScale;

        rigidBody.forceRequests.impulses.push_back(Force{ .value = { impulse, 0.0f } });

        return ReturnSignal::KeepObserving;
    };
}

} // test