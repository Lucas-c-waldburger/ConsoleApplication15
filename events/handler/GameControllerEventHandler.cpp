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

void GameControllerEventHandler::HandleDeviceEvent(const SDL_Event& ev)
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

		EventBus::PushEvent(events::GameControllerConnected{ .joystickID = joystickId });

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

		EventBus::PushEvent(events::GameControllerDisconnected{ .joystickID = deadJoystickId });

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
		LOG_ERROR("Controller active but wasn't properly connected\n");
		return;
	}

	auto& inputUpdater = it->second.second;

	inputUpdater.Update(ev);
}

//void GameControllerEventHandler::HandleInputEvent(const SDL_Event& ev)
//{
//	auto getAffectedAxisSide = [](auto axisType, auto& axisInputPair) -> AxisInputData& {
//		auto& [left, right] = axisInputPair;
//		return (axisType == SDL_CONTROLLER_AXIS_LEFTX ||
//				axisType == SDL_CONTROLLER_AXIS_LEFTY) ? left : right;
//	};
//	auto getAffectedAxisValue = [](auto axisType, auto& axisInputSide) -> float& {
//		auto& [x, y] = axisInputSide.value;
//		return (axisType == SDL_CONTROLLER_AXIS_LEFTX ||
//			    axisType == SDL_CONTROLLER_AXIS_RIGHTX) ? x : y;
//	};
//
//	switch (ev.type)
//	{
//	case SDL_CONTROLLERAXISMOTION:
//	{
//		if (!activeControllers_.contains(ev.caxis.which))
//		{
//			LOG_ERROR("Controller active but wasn't properly connected\n");
//			return;
//		}
//
//		auto& lastInput = activeControllers_[ev.caxis.which].second.cachedControllerState.axisInput;
//		auto& lastInputSide = getAffectedAxisSide(ev.caxis.axis, lastInput);
//
//		auto& xOrY = getAffectedAxisValue(ev.caxis.axis, lastInputSide);
//
//		//bool inDeadzone = (std::abs(ev.caxis.value) <= GameController::kAxisDeadzone);
//		//xOrY = (inDeadzone) ? 0.0f : static_cast<float>(ev.caxis.value);
//
//		xOrY = static_cast<float>(ev.caxis.value);
//
//		bool markAsHeld = (lastInputSide.state == InputState::Pressed ||
//						   lastInputSide.state == InputState::Held);
//		if (markAsHeld)
//		{
//			lastInputSide.state = InputState::Held;
//			lastInputSide.stateDuration = lastInputSide.stateDuration +
//				(ev.caxis.timestamp - lastInputSide.timestamp);
//		}
//		else
//		{
//			lastInputSide.state = InputState::Pressed;
//			lastInputSide.stateDuration = 0;
//		}
//
//		lastInputSide.timestamp = ev.caxis.timestamp;
//
//		uint8_t cacheAxisIndex = InputDataCache::GetAxisIndexForEnum(ev.caxis.axis);
//		activeControllers_[ev.caxis.which].second.inputUpdatedTracker.set(cacheAxisIndex, true);
//
//		break;
//	}
//	case SDL_CONTROLLERBUTTONDOWN: case SDL_CONTROLLERBUTTONUP:
//	{
//		if (!activeControllers_.contains(ev.cbutton.which))
//		{
//			std::cout << "Controller active but wasn't properly connected\n";
//			return;
//		}
//
//		ButtonInputData newButtonInput{};
//		newButtonInput.button = static_cast<SDL_GameControllerButton>(ev.cbutton.button);
//		newButtonInput.timestamp = ev.cbutton.timestamp;
//
//		auto& inputCache = activeControllers_[ev.cbutton.which].second;
//		auto& lastButtonInput = inputCache.cachedControllerState.buttonInput[ev.cbutton.button];
//
//
//		newButtonInput.state = (ev.type == SDL_CONTROLLERBUTTONUP) ? InputState::Released :
//																	 InputState::Pressed;
//		newButtonInput.stateDuration = 0;
//
//		lastButtonInput = std::move(newButtonInput);
//		inputCache.inputUpdatedTracker.set(ev.cbutton.button, true);
//
//		break;
//	}
//	default:
//		break;
//	}
//}

void GameControllerEventHandler::Finalize()
{
	for (auto& [joystickId, pair] : activeControllers_)
	{
		auto& updater = pair.second;

		updater.FinalizeAndPushEvents(joystickId);
	}

	UpdateControllerStateComponents();
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
