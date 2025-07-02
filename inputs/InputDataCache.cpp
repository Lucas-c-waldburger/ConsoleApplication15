#include "InputDataCache.h"
//#include "InputState.h"
//
//// TODO : Set axis state to NONE if no motion event this frame & axis values < kDeadzone
//void InputDataCache::UpdateSkippedInputs()
//{
//	uint64_t now = SDL_GetTicks64();
//
//	auto updateFromLastState = [now](auto& inputData) {
//		switch (inputData.state)
//		{
//		case InputState::Pressed:
//		case InputState::Held:
//			inputData.state = InputState::Held;
//			break;
//
//		case InputState::Released:
//		case InputState::None:
//		default:
//			inputData.state = InputState::None;
//			break;
//		}
//
//		inputData.stateDuration = static_cast<uint32_t>(now) - inputData.timestamp;
//	};
//
//	for (uint8_t i = 0; i < inputUpdatedTracker.size(); i++)
//	{
//		if (!inputUpdatedTracker.test(i))
//		{
//			switch (i)
//			{
//			case LeftAxisIndex:
//				updateFromLastState(cachedControllerState.axisInput.left);
//				break;
//			case RightAxisIndex:
//				updateFromLastState(cachedControllerState.axisInput.right);
//				break;
//			default:
//				updateFromLastState(cachedControllerState.buttonInput[i]);
//				break;
//			}
//		}
//	}
//
//	inputUpdatedTracker.reset();
//}
//
//uint8_t InputDataCache::GetAxisIndexForEnum(uint8_t axisEnum)
//{
//	return (axisEnum == SDL_CONTROLLER_AXIS_LEFTX ||
//			axisEnum == SDL_CONTROLLER_AXIS_LEFTY) ? LeftAxisIndex : RightAxisIndex;
//}