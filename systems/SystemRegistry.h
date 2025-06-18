#pragma once
#include "CameraSystem.h"
#include "CollisionSystem.h"
#include "PhysicsSystem.h"
#include "RenderSystem.h"
#include "SDLInputSystem.h"
#include "EventCallbackSystem.h"

#define SYSTEM_REGISTRY \
	CameraSystem, \
	PhysicsSystem, \
	RenderSystem, \
	SDLInputSystem, \
	EventCallbackSystem

