#pragma once
#include "ComponentHelper.h"
#include "../GameControllerStateComponent.h"
#include "../../inputs/controller/GameController.h"


template <>
class ComponentHelper<GameControllerState>
{
public:

	explicit ComponentHelper(GameControllerState& state) : controllerState_(state) {}

	ComponentHelper(const ComponentHelper&) = delete;
	ComponentHelper(ComponentHelper&&) = delete;
	ComponentHelper& operator=(const ComponentHelper&) = delete;
	ComponentHelper& operator=(ComponentHelper&&) = delete;

	SDL_FPoint GetLeftAxisNormal() const
	{
		return controllerState_.axisInput.left.value / GameController::kAxisMax;
	}

private:
	GameControllerState& controllerState_;

};