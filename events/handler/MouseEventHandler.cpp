#include "MouseEventHandler.h"
#include "../../inputs/InputState.h"
#include "../../ecs/Ecs.h"

void MouseEventHandler::HandleMotionEvent(const SDL_Event& ev)
{
	assert(ev.type == SDL_MOUSEMOTION);

	auto& lastInput = inputCache.data.positionData;

	lastInput.value.x = ev.motion.x;
	lastInput.value.y = ev.motion.y;

	// TODO : Pull this out into function used by game controller event handler too
	if (lastInput.state == InputState::None)
	{
		lastInput.state = InputState::Pressed;
		lastInput.stateDuration = 0;
	}
	else if (lastInput.state == InputState::Held)
	{
		lastInput.stateDuration += (ev.motion.timestamp - lastInput.timestamp);
	}

	lastInput.timestamp = ev.motion.timestamp;

	inputCache.MarkUpdated(InputCache::kMousePositionShiftedIndex);
}

void MouseEventHandler::HandleButtonEvent(const SDL_Event& ev)
{
	assert(ev.type == SDL_MOUSEBUTTONDOWN || ev.type == SDL_MOUSEBUTTONUP);

	auto& lastInput = inputCache.data.buttonData[ev.button.button];

	lastInput.timestamp = ev.button.timestamp;
	lastInput.stateDuration = 0;
	lastInput.state = (ev.type == SDL_MOUSEBUTTONDOWN) ? InputState::Pressed : InputState::Released;

	inputCache.MarkUpdated(ev.button.button);
}

void MouseEventHandler::UpdateEntities()
{
	inputCache.UpdateSkippedInputs();

	auto entities = ECS::GetAllEntitiesWith<MouseState>();

	for (auto& entity : entities)
	{
		auto& mouseState = entity.GetComponent<MouseState>();

		mouseState = inputCache.data;
	}
}

void MouseEventHandler::InputCache::MarkUpdated(size_t shiftedIdx)
{
	size_t actualIdx = shiftedIdx - 1;
	assert(actualIdx < updatedTracker.size());

	updatedTracker.set(actualIdx, true);
}

void MouseEventHandler::InputCache::UpdateSkippedInputs()
{
	uint64_t now = SDL_GetTicks64();

	auto updateFromLastState = [now](auto& inputData) {
		switch (inputData.state)
		{
		case InputState::Pressed:
		case InputState::Held:
			inputData.state = InputState::Held;
			break;

		case InputState::Released:
		case InputState::None:
		default:
			inputData.state = InputState::None;
			break;
		}

		inputData.stateDuration = static_cast<uint32_t>(now) - inputData.timestamp;
	};

	for (uint8_t i = 0; i < updatedTracker.size(); i++)
	{
		if (!updatedTracker.test(i))
		{
			size_t shiftedIdx = i + 1;

			switch (shiftedIdx)
			{
			case kMousePositionShiftedIndex:
				updateFromLastState(data.positionData);
				break;
			default:
				updateFromLastState(data.buttonData[shiftedIdx]);
				break;
			}
		}
	}

	updatedTracker.reset();
}
