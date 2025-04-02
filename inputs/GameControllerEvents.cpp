#include "GameControllerEvents.h"
#include "../ecs/Ecs.h"
#include "../events/CustomEventDataRegistry.h"
#include "../core/ScopedInvoker.h"
#include <cassert>

namespace
{
Result<Void> NotifyControllerConnected(SDL_JoystickID newConnection)
{
	auto entities = ECS::GetAllEntitiesWith<EventObserver>([](const EventObserver& events) {
		auto it = events.eventCallbacks.find(GameControllerConnected::GetEventType());

		return it != events.eventCallbacks.end() && 
			   it->second.func && 
			   it->second.status != ReturnSignal::Pause;
	});

	if (entities.empty())
	{
		return Void{};
	}

	auto connectedEv = CustomEvents::MakeNewEvent(GameControllerConnected{ .joystickID = newConnection });
	ScopedInvoker cleanup{[&connectedEv]() {
		LOG_IF_ERROR(CustomEvents::FreeEvent<GameControllerConnected>(connectedEv));
	}};

	assert(connectedEv.user.data1 != nullptr);

	for (auto& entity : entities)
	{
		auto& events = entity.GetComponent<EventObserver>();
		auto& [func, status] = events.eventCallbacks[GameControllerConnected::GetEventType()];

		status = func(connectedEv, entity);
		if (status == ReturnSignal::StopObserving)
		{
			events.eventCallbacks.erase(GameControllerConnected::GetEventType());
		}
	}

	return Void{};
}

} // unnamed namespace

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

		bool inDeadzone = (std::abs(ev.caxis.value) <= GameController::kAxisDeadzone);
		xOrY = (inDeadzone) ? 0.0f : static_cast<float>(ev.caxis.value);

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

std::set<SDL_JoystickID> GameControllerEventHandler::GetConnectedControllerIDs() const
{
	std::set<SDL_JoystickID> availableIDs{};
	for (const auto& [id, _] : activeControllers_)
	{
		availableIDs.insert(id);
	}

	return availableIDs;
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