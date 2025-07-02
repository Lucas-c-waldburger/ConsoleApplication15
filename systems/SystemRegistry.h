#pragma once
#include "CameraSystem.h"
#include "CollisionSystem.h"
#include "PhysicsSystem.h"
#include "RenderSystem.h"
#include "SDLInputSystem.h"
#include "EventCallbackSystem.h"
#include "TimerSystem.h"
#include "SpriteAnimationSystem.h"

// TODO: Either make into type list or dont require strong typing like this for systems

#define SYSTEM_REGISTRY \
	CameraSystem, \
	PhysicsSystem, \
	SDLInputSystem, \
	EventCallbackSystem, \
	RenderSystem, \
	TimerSystem, \
	SpriteAnimationSystem

