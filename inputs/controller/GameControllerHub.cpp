#include "GameControllerHub.h"

GameControllerHub::~GameControllerHub()
{
	for (auto& controller : gameControllers_)
	{
		controller.Disconnect();
	}
}

GameControllerHub::GameControllerHub(GameControllerHub&& other) noexcept :
	joystickIdToControllerIndex_(std::move(other.joystickIdToControllerIndex_)),
	gameControllers_(std::move(other.gameControllers_)), 
	controllerStates_(std::move(other.controllerStates_))
{
	other.gameControllers_.clear();
}

GameControllerHub& GameControllerHub::operator=(GameControllerHub&& other) noexcept
{
	if (this != &other)
	{
		joystickIdToControllerIndex_ = std::move(other.joystickIdToControllerIndex_);
		gameControllers_ = std::move(other.gameControllers_);
		controllerStates_ = std::move(other.controllerStates_);

		other.gameControllers_.clear();
	}
	return *this;
}

Result<SDL_JoystickID> GameControllerHub::Connect(Sint32 deviceIdx)
{
	size_t vecIdx = gameControllers_.size();

	auto& newController = gameControllers_.emplace_back();
	auto connectResult = newController.Connect(deviceIdx);
	if (!connectResult.Success())
	{
		gameControllers_.pop_back();
		return connectResult.GetError();
	}

	const SDL_JoystickID newJoystickId = connectResult.GetValue();
	joystickIdToControllerIndex_.try_emplace(newJoystickId, vecIdx);
	controllerStates_.emplace_back(GameControllerState{ .joystickID = newJoystickId });

	return newJoystickId;
}

bool GameControllerHub::Disconnect(SDL_JoystickID joystickId)
{
	auto it = joystickIdToControllerIndex_.find(joystickId);
	if (it == joystickIdToControllerIndex_.end())
	{
		return false;
	}

	const size_t idx = it->second;

	assert(idx < gameControllers_.size());
	assert(controllerStates_.size() == gameControllers_.size());
	assert(gameControllers_[idx].GetJoystickID() == joystickId);

	if (idx != gameControllers_.size() - 1)
	{
		SDL_JoystickID backJoystickId = gameControllers_.back().GetJoystickID();
		assert(joystickIdToControllerIndex_.contains(backJoystickId));

		std::swap(gameControllers_[idx], gameControllers_.back());
		std::swap(controllerStates_[idx], controllerStates_.back());

		joystickIdToControllerIndex_[backJoystickId] = idx;
	}

	gameControllers_.back().Disconnect();

	gameControllers_.pop_back();
	controllerStates_.pop_back();

	joystickIdToControllerIndex_.erase(joystickId);

	return true;
}

bool GameControllerHub::IsConnected(SDL_JoystickID joystickId) const
{
	auto it = joystickIdToControllerIndex_.find(joystickId);
	if (it == joystickIdToControllerIndex_.end())
	{
		return false;
	}

	assert(it->second < gameControllers_.size());

	return gameControllers_[it->second].IsConnected();
}

const GameControllerState& 
GameControllerHub::GetControllerState(SDL_JoystickID joystickId) const
{
	auto it = joystickIdToControllerIndex_.find(joystickId);
	if (it == joystickIdToControllerIndex_.end())
	{
		return kInvalidGameControllerState;
	}

	assert(it->second < controllerStates_.size());
	assert(controllerStates_[it->second].joystickID == joystickId);

	return controllerStates_[it->second];
}
