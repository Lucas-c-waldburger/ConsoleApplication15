#include "GameControllerEventHandler.h"
#include "../../inputs/InputState.h"
#include "../../components/util/EventObserverUtils.h"
#include "../../events/custom/CustomEventDataRegistry.h"
#include <cassert>

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

		LOG_IF_ERROR(SendEventNotification(GameControllerConnected{ .joystickID = joystickId }));

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

		LOG_IF_ERROR(SendEventNotification(GameControllerDisconnected{ .joystickID = deadJoystickId }));

		break;
	}
	default:
		break;
	}
}

void GameControllerEventHandler::HandleInputEvent(const SDL_Event& ev)
{
	auto getAffectedAxisSide = [](auto axisType, auto& axisInputPair) -> AxisInputData& {
		auto& [left, right] = axisInputPair;
		return (axisType == SDL_CONTROLLER_AXIS_LEFTX ||
				axisType == SDL_CONTROLLER_AXIS_LEFTY) ? left : right;
	};
	auto getAffectedAxisValue = [](auto axisType, auto& axisInputSide) -> float& {
		auto& [x, y] = axisInputSide.value;
		return (axisType == SDL_CONTROLLER_AXIS_LEFTX ||
			    axisType == SDL_CONTROLLER_AXIS_RIGHTX) ? x : y;
	};

	switch (ev.type)
	{
	case SDL_CONTROLLERAXISMOTION:
	{
		//LOG_DEBUG("Handling controller axis motion inside GameControllerEventHandler::HandleInputEvent");
		if (!activeControllers_.contains(ev.caxis.which))
		{
			LOG_ERROR("Controller active but wasn't properly connected\n");
			return;
		}

		auto& lastInput = activeControllers_[ev.caxis.which].second.cachedControllerState.axisInput;
		auto& lastInputSide = getAffectedAxisSide(ev.caxis.axis, lastInput);

		auto& xOrY = getAffectedAxisValue(ev.caxis.axis, lastInputSide);

		//bool inDeadzone = (std::abs(ev.caxis.value) <= GameController::kAxisDeadzone);
		//xOrY = (inDeadzone) ? 0.0f : static_cast<float>(ev.caxis.value);

		xOrY = static_cast<float>(ev.caxis.value);

		bool markAsHeld = (lastInputSide.state == InputState::Pressed ||
						   lastInputSide.state == InputState::Held);
		if (markAsHeld)
		{
			lastInputSide.state = InputState::Held;
			lastInputSide.stateDuration = lastInputSide.stateDuration +
				(ev.caxis.timestamp - lastInputSide.timestamp);
		}
		else
		{
			lastInputSide.state = InputState::Pressed;
			lastInputSide.stateDuration = 0;
		}

		lastInputSide.timestamp = ev.caxis.timestamp;

		//tracking_ = lastInputSide;

		uint8_t cacheAxisIndex = InputDataCache::GetAxisIndexForEnum(ev.caxis.axis);
		activeControllers_[ev.caxis.which].second.inputUpdatedTracker.set(cacheAxisIndex, true);

		break;
	}
	case SDL_CONTROLLERBUTTONDOWN: case SDL_CONTROLLERBUTTONUP:
	{
		if (!activeControllers_.contains(ev.cbutton.which))
		{
			std::cout << "Controller active but wasn't properly connected\n";
			return;
		}

		ButtonInputData newButtonInput{};
		newButtonInput.button = static_cast<SDL_GameControllerButton>(ev.cbutton.button);
		newButtonInput.timestamp = ev.cbutton.timestamp;

		auto& inputCache = activeControllers_[ev.cbutton.which].second;
		auto& lastButtonInput = inputCache.cachedControllerState.buttonInput[ev.cbutton.button];


		newButtonInput.state = (ev.type == SDL_CONTROLLERBUTTONUP) ? InputState::Released :
																	 InputState::Pressed;
		newButtonInput.stateDuration = 0;

		lastButtonInput = std::move(newButtonInput);
		inputCache.inputUpdatedTracker.set(ev.cbutton.button, true);

		break;
	}
	default:
		break;
	}
}

void GameControllerEventHandler::UpdateEntities()
{
	//LOG_INFO(tracking_, '\n');

	for (auto& [_, controllerCachePair] : activeControllers_)
	{
		controllerCachePair.second.UpdateSkippedInputs();
	}

	auto controllerEntities = ECS::GetAllEntitiesWith<GameControllerState>(
		[](const GameControllerState& cmp) { 
			return cmp.joystickID != GameController::kInvalidJoystickID; 
		});

	for (auto& entity : controllerEntities)
	{
		auto& componentControllerState = entity.GetComponent<GameControllerState>();

		auto it = activeControllers_.find(componentControllerState.joystickID);
		if (it == activeControllers_.end())
		{
			std::cout << "Controller with JoystickID '" <<
				componentControllerState.joystickID << "' not connected\n";

			componentControllerState.joystickID = GameController::kInvalidJoystickID;
			continue;
		}

		auto& cachedControllerState = it->second.second.cachedControllerState;

		componentControllerState.axisInput = cachedControllerState.axisInput;
		componentControllerState.buttonInput = cachedControllerState.buttonInput;
	}
}

//auto [x, y] = cachedControllerState.axisInput.left.value;
//if (x > 0.0f || y > 0.0f)
//{
//	LOG_INFO_FMT("From UpdateEntities: [ {}, {} ]", x, y);
//}