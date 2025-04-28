#pragma once
#include "../components/ComponentRegistry.h"
#include "../ecs/Ecs.h"


static void HandleControllerMovementBehavior(Entity& entity)
{
	//auto [controller, physics, spatial] = 
	//	entity.GetComponents<GameControllerState, Physics, Spatial>();

 //   SDL_FPoint moveForce = { 0.0f, 0.0f };
 //   moveForce.x = controller.axisInput.left.value.x /
 //       static_cast<float>(GameController::kAxisMax) * physics.forces.max;

 //   bool hasJumped = controller.buttonInput[SDL_CONTROLLER_BUTTON_A].state == GameControllerState::Pressed;
 //   //if (hasJumped)

 //   SDL_FPoint normed = {
 //       controller.axisInput.left.value.x / static_cast<float>(GameController::kAxisMax) * physics.forces.max,
 //       controller.axisInput.left.value.y / static_cast<float>(GameController::kAxisMax) * physics.forces.max
 //   };

 //   physics.forces.normed.push_back(Force{ .vector = normed, .duration = 0 });
}