#include "GameController.h"


Result<SDL_JoystickID> GameController::Connect(Sint32 deviceIndex)
{
	return ConnectImpl(deviceIndex);
}

Result<SDL_JoystickID> GameController::ConnectFirstAvailable()
{
	return ConnectImpl(FindControllerIndex());
}

void GameController::Disconnect()
{
	if (IsConnected())
	{
		SDL_GameControllerClose(controller_);

		controller_ = nullptr;
		joystickId_ = kInvalidJoystickID;
	}
}

bool GameController::IsConnected() const
{
	return controller_ && joystickId_ > kInvalidJoystickID &&
		SDL_GameControllerGetAttached(controller_);
}


Result<SDL_JoystickID> GameController::ConnectImpl(Sint32 deviceIndex)
{
	controller_ = SDL_GameControllerOpen(deviceIndex);
	if (!controller_)
	{
		return MAKE_ERROR("No controller found");
	}

	SDL_Joystick* joystick = SDL_GameControllerGetJoystick(controller_);
	if (!joystick)
	{
		return MAKE_ERROR(SDL_GetError());
	}

	joystickId_ = SDL_JoystickInstanceID(joystick);
	if (joystickId_ < 0)
	{
		return MAKE_ERROR(SDL_GetError());
	}

	return joystickId_;
}

Sint32 GameController::FindControllerIndex()
{
	for (int i = 0; i < SDL_NumJoysticks(); i++)
	{
		if (SDL_IsGameController(i))
		{
			return i;
		}
	}
}