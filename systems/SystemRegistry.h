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
#include "SerializationSystem.h"

#if IMGUI_ENABLED
#include "GuiSystem.h"
#endif

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
	SerializationSystem,

#if IMGUI_ENABLED
	GuiSystem
#endif

>;

static_assert(unique_type_list_v<SystemTypeList>);