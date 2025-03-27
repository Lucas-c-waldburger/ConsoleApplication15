#include "GameControllerEvents.h"
#include "../Ecs.h"

// TODO : Set axis state to NONE if no motion event this frame & axis values < kDeadzone
void GameControllerEventHandler::InputDataCache::UpdateSkippedInputs()
{
	uint64_t now = SDL_GetTicks64();

	auto updateFromLastState = [now](auto& inputData) {
		switch (inputData.state)
		{
		case GameControllerState::Pressed:
		case GameControllerState::Held:
			inputData.state = GameControllerState::Held;
			break;

		case GameControllerState::Released:
		case GameControllerState::None:
		default:
			inputData.state = GameControllerState::None;
			break;
		}

		inputData.stateDuration = now - inputData.timestamp;
	};

	for (uint8_t i = 0; i < inputUpdatedTracker.size(); i++)
	{
		if (!inputUpdatedTracker.test(i))
		{
			switch (i)
			{
			case LeftAxisIndex:
				updateFromLastState(cachedControllerState.axisInput.left);
				break;
			case RightAxisIndex:
				updateFromLastState(cachedControllerState.axisInput.right);
				break;
			default:
				updateFromLastState(cachedControllerState.buttonInput[i]);
				break;
			}
		}
	}

	inputUpdatedTracker.reset();
}

uint8_t GameControllerEventHandler::InputDataCache::GetAxisIndexForEnum(uint8_t axisEnum)
{
	return (axisEnum == SDL_CONTROLLER_AXIS_LEFTX ||
		axisEnum == SDL_CONTROLLER_AXIS_LEFTY) ? LeftAxisIndex : RightAxisIndex;
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

		break;
	}
	case SDL_CONTROLLERDEVICEREMOVED:
	{
		LOG_INFO("Controller disconnected");

		assert(activeControllers_.contains(ev.cdevice.which));

		auto& deadController = activeControllers_[ev.cdevice.which].first;
		deadController.Disconnect();

		activeControllers_.erase(ev.cdevice.which);

		break;
	}
	default:
		break;
	}
}

void GameControllerEventHandler::HandleInputEvent(const SDL_Event& ev)
{
	auto getAxisInputSide = [](auto axisType, auto& axisInputPair) -> AxisInputState& {
		return (axisType == SDL_CONTROLLER_AXIS_LEFTX ||
				axisType == SDL_CONTROLLER_AXIS_LEFTY) ?
			axisInputPair.left : axisInputPair.right;
	};
	auto getAxisValue = [](auto axisType, auto& axisInputSide) -> float& {
		return (axisType == SDL_CONTROLLER_AXIS_LEFTX ||
				axisType == SDL_CONTROLLER_AXIS_RIGHTX) ?
			axisInputSide.value.x : axisInputSide.value.y;
	};

	switch (ev.type)
	{
	case SDL_CONTROLLERAXISMOTION:
	{
		if (!activeControllers_.contains(ev.caxis.which))
		{
			std::cout << "Controller active but wasn't properly connected\n";
			return;
		}

		AxisInputState newAxisInput{};
		newAxisInput.timestamp = ev.caxis.timestamp;

		auto& xOrY = getAxisValue(ev.caxis.axis, newAxisInput);
		xOrY = (ev.caxis.value > GameController::kAxisDeadzone) ? ev.caxis.value : 0;

		auto& lastInput = activeControllers_[ev.caxis.which].second.cachedControllerState.axisInput;
		auto& lastInputSide = getAxisInputSide(ev.caxis.axis, lastInput);

		bool markAsHeld = (lastInputSide.state == GameControllerState::Pressed ||
						   lastInputSide.state == GameControllerState::Held);
		if (markAsHeld)
		{
			newAxisInput.state = GameControllerState::Held;
			newAxisInput.stateDuration = lastInputSide.stateDuration +
				(newAxisInput.timestamp - lastInputSide.timestamp);
		}
		else
		{
			newAxisInput.state = GameControllerState::Pressed;
			newAxisInput.stateDuration = 0;
		}

		lastInputSide = std::move(newAxisInput);

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

		ButtonInputState newButtonInput{};
		newButtonInput.button = static_cast<SDL_GameControllerButton>(ev.cbutton.button);
		newButtonInput.timestamp = ev.cbutton.timestamp;

		auto& inputCache = activeControllers_[ev.cbutton.which].second;
		auto& lastButtonInput = inputCache.cachedControllerState.buttonInput[ev.cbutton.button];


		newButtonInput.state = (ev.type == SDL_CONTROLLERBUTTONUP) ? GameControllerState::Released :
			GameControllerState::Pressed;
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
	for (auto& [_, controllerCachePair] : activeControllers_)
	{
		controllerCachePair.second.UpdateSkippedInputs();
	}

	auto controllerEntities = ECS::GetAllEntitiesWith<GameControllerState>(
		[](const GameControllerState& cmp) { return cmp.joystickID != GameController::kInvalidJoystickID; }
	);

	for (auto& entity : controllerEntities)
	{
		auto& controllerState = entity.GetComponent<GameControllerState>();

		auto it = activeControllers_.find(controllerState.joystickID);
		if (it == activeControllers_.end())
		{
			std::cout << "Controller with JoystickID '" <<
				controllerState.joystickID << "' not connected\n";

			controllerState.joystickID = GameController::kInvalidJoystickID;
			continue;
		}

		controllerState = it->second.second.cachedControllerState;
	}
}

std::ostream& operator<<(std::ostream& os, const GameControllerState::State& st)
{
	using ST = GameControllerState::State;
	os << (st == ST::Pressed) ? "Pressed" : (st == ST::Released) ? "Released" :
		(st == ST::Held) ? "Held" : "None";

	return os;
}
std::ostream& operator<<(std::ostream& os, const AxisInputState& inp)
{
	os << "XY: { " << inp.value.x << ", " << inp.value.y << " }\nTimestamp: "
		<< inp.timestamp << "\nState: " << inp.state << "\nStateDuration: "
		<< inp.stateDuration << "\n\n";

	return os;
}
std::ostream& operator<<(std::ostream& os, const ButtonInputState& inp)
{
	os << "Button: " << SDL_GameControllerGetStringForButton(inp.button) << "\nTimestamp: "
		<< inp.timestamp << "\nState: " << inp.state << "\nStateDuration: "
		<< inp.stateDuration << "\n\n";

	return os;
}