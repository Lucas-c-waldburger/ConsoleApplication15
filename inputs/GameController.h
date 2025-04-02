#pragma once
#include <SDL.h>
#include "../core/Result.h"

class GameController
{
public:
	static constexpr SDL_JoystickID kInvalidJoystickID = -1;
	static constexpr int kAxisDeadzone = 1700;
	static constexpr int kAxisMax = 32768;

	GameController() = default;

	Result<SDL_JoystickID> Connect(Sint32 deviceIndex);
	Result<SDL_JoystickID> ConnectFirstAvailable();
	void Disconnect();
	bool IsConnected() const;

	SDL_JoystickID GetJoystickID() const { return joystickId_; }

private:
	Result<SDL_JoystickID> ConnectImpl(Sint32 deviceIndex);

	static Sint32 FindControllerIndex();

	SDL_GameController* controller_ = nullptr;
	SDL_JoystickID joystickId_ = kInvalidJoystickID;
};









//class GameController
//{
//public:
//	GameController() = default;
//	~GameController() { Disconnect(); }
//
//
//
//	Result<Void> Connect()
//	{
//		controller_ = FindController();
//		if (!controller_)
//		{
//			return MAKE_ERROR("No controller found", Severity::Warn);
//		}
//
//		SDL_GameController
//
//		return Void{};
//	}
//
//	void Disconnect()
//	{
//		if (controller_) 
//		{
//			SDL_GameControllerClose(controller_);
//			controller_ = nullptr;
//		}
//	}
//
//	SDL_Point GetLeftAxis() const
//	{
//		if (!controller_) { return { -1, -1 }; }
//
//		Sint16 xAxis = SDL_GameControllerGetAxis(controller_, SDL_CONTROLLER_AXIS_LEFTX);
//		Sint16 yAxis = SDL_GameControllerGetAxis(controller_, SDL_CONTROLLER_AXIS_LEFTY);
//
//		return { xAxis, yAxis };
//	}
//
//	bool IsConnected() const { return controller_; }
//
//	SDL_Point operator[](const Axis axis)
//	{
//		if (!controller_) { return { -1, -1 }; }
//
//		SDL_GameControllerAxis x = (axis == Axis::Left) ? SDL_CONTROLLER_AXIS_LEFTX :
//														  SDL_CONTROLLER_AXIS_RIGHTX;
//		SDL_GameControllerAxis y = (axis == Axis::Left) ? SDL_CONTROLLER_AXIS_LEFTY :
//														  SDL_CONTROLLER_AXIS_RIGHTY;
//
//		return { SDL_GameControllerGetAxis(controller_, x),
//				 SDL_GameControllerGetAxis(controller_, y) };
//	}
//
//
//
//private:
//	static SDL_GameController* FindController()
//	{
//		for (int i = 0; i < SDL_NumJoysticks(); i++)
//		{
//			if (SDL_IsGameController(i))
//			{
//				return SDL_GameControllerOpen(i);
//			}
//		}
//
//		return nullptr;
//	}
//
//	SDL_GameController* controller_ = nullptr;
//	SDL_JoystickGUID guid_ = {};
//};

