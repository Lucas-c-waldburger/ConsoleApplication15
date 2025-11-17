#pragma once
#include "CameraSystem.h"
#include "CollisionSystem.h"
#include "PhysicsSystem.h"
#include "NewRenderSystem.h"
#include "SDLInputSystem.h"
#include "EventCallbackSystem.h"
#include "TimerSystem.h"
#include "SpriteAnimationSystem.h"
#include "EntityStateSystem.h"
#include "GameLoopSystem.h"
#include "AudioSystem.h"

// TODO: Either make into type list or dont require strong typing like this for system managing

#define SYSTEM_REGISTRY_LIST \
	CameraSystem, \
	PhysicsSystem, \
	SDLInputSystem, \
	EventCallbackSystem, \
	TimerSystem, \
	SpriteAnimationSystem, \
	EntityStateSystem, \
	GameLoopSystem, \
	AudioSystem, \
	NewRenderSystem


using SystemTypeList = TypeList<
	CameraSystem, 
	PhysicsSystem, 
	SDLInputSystem, 
	EventCallbackSystem, 
	TimerSystem, 
	SpriteAnimationSystem, 
	EntityStateSystem,
	GameLoopSystem,
	AudioSystem,
	NewRenderSystem
>;

