#pragma once
#include "BaseComponent.h"
#include <SDL_rect.h>


//struct CameraTarget : public BaseComponent<CameraTarget, 12>
struct CameraTarget : public BaseComponent<CameraTarget>
{
	SDL_FPoint offset = { 0.0f, 0.0f };
	float followSpeed = 5.0f;
};