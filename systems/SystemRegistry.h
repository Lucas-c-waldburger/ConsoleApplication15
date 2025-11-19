#pragma once
#include "../FeatureFlags.h"
#include "../core/TypeUtils.h"
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

#if IMGUI_ENABLED
#include "GuiSystem.h"
#endif

// TODO: Either make into type list or dont require strong typing like this for system managing

//#define SYSTEM_REGISTRY_LIST \
//	CameraSystem, \
//	PhysicsSystem, \
//	SDLInputSystem, \
//	EventCallbackSystem, \
//	TimerSystem, \
//	SpriteAnimationSystem, \
//	EntityStateSystem, \
//	GameLoopSystem, \
//	AudioSystem, \
//	NewRenderSystem, \
//	GuiSystem


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
	NewRenderSystem,
#if IMGUI_ENABLED
	GuiSystem
#endif
>;

static_assert(unique_type_list_v<SystemTypeList>);