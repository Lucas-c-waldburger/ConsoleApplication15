#include "GameControllerEventHandler.h"
#include "../../inputs/InputState.h"
#include "../../events/EventBus.h"
#include "../../ecs/Ecs.h"
#include "../data/GameControllerEvents.h"
#include <cassert>

namespace {

constexpr SDL_JoystickID ExtractJoystickIDFromEvent(const SDL_Event& ev)
{
	switch (ev.type)
	{
	case SDL_CONTROLLERAXISMOTION:
		return ev.caxis.which;
	case SDL_CONTROLLERBUTTONDOWN: case SDL_CONTROLLERBUTTONUP:
		return ev.cbutton.which;
	default:
		return GameController::kInvalidJoystickID;
	}
}

} // unnamed

GameControllerEventHandler::~GameControllerEventHandler()
{
	for (auto& [_, controllerInputPair] : activeControllers_)
	{
		controllerInputPair.first.Disconnect();
	}
}

void GameControllerEventHandler::HandleDeviceEvent(const SDL_Event& ev, EventBus2& bus)
{
	switch (ev.type)
	{
	case SDL_CONTROLLERDEVICEADDED:
	{
		LOG_INFO("New controller detected");

		GameController newController{};

		auto connectResult = newController.Connect(ev.cdevice.which);
		if (!connectResult.Success())
		{
			LOG_WARNING("Could not open controller");
			return;
		}

		LOG_INFO("Controller successfully connected!");

		auto joystickId = connectResult.GetValue();
		assert(!activeControllers_.contains(joystickId));

		activeControllers_[joystickId].first = std::move(newController);

		bus.PushEvent(events::GameControllerConnected{ 
			.joystickID = joystickId 
		});

		break;
	}
	case SDL_CONTROLLERDEVICEREMOVED:
	{
		LOG_INFO("Controller disconnected");

		assert(activeControllers_.contains(ev.cdevice.which));

		auto& deadController = activeControllers_[ev.cdevice.which].first;
		auto deadJoystickId = deadController.GetJoystickID();

		deadController.Disconnect();

		activeControllers_.erase(ev.cdevice.which);

		bus.PushEvent(events::GameControllerDisconnected{ 
			.joystickID = deadJoystickId 
		});

		break;
	}
	default:
		break;
	}
}

void GameControllerEventHandler::HandleInputEvent(const SDL_Event& ev)
{
	SDL_JoystickID joystickId = ExtractJoystickIDFromEvent(ev);

	if (joystickId == -1)
	{
		return;
	}

	auto it = activeControllers_.find(joystickId);
	if (it == activeControllers_.end())
	{
		LOG_ERROR("Controller active but wasn't properly connected");
		return;
	}

	auto& inputUpdater = it->second.second;

	inputUpdater.Update(ev);
}

void GameControllerEventHandler::Finalize(EventBus2& bus)
{
	for (auto& [joystickId, pair] : activeControllers_)
	{
		auto& [gc, updater] = pair;

		updater.FinalizeAndPushEvents(joystickId, gc, bus);
	}

	UpdateControllerStateComponents();
}

GameControllerState
GameControllerEventHandler::GetControllerState(SDL_JoystickID joystickId) const
{
	auto it = activeControllers_.find(joystickId);
	if (it == activeControllers_.end())
	{
		return { .joystickID = -1 };
	}

	return {
		.joystickID = joystickId,
		.inputs = it->second.second.GetInputMap()
	};
}

void GameControllerEventHandler::UpdateControllerStateComponents()
{
	auto entities = ECS::GetAllEntitiesWith<GameControllerState>();

	for (auto& entity : entities)
	{
		auto& controllerState = entity.GetComponent<GameControllerState>();

		if (controllerState.joystickID == GameController::kInvalidJoystickID)
		{
			continue;
		}

		auto it = activeControllers_.find(controllerState.joystickID);
		if (it == activeControllers_.end())
		{
			LOG_ERROR_FMT("Controller with JoystickID '{}' not connected",
				controllerState.joystickID);

			controllerState.joystickID = GameController::kInvalidJoystickID;

			continue;
		}

		controllerState.inputs = it->second.second.GetInputMap();
	}
}
