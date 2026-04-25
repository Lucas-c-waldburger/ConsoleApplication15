#pragma once
#include "../FeatureFlags.h"
#include "../core/TypeUtils.h"
#include "CameraSystem.h"
#include "CollisionSystem.h"
#include "PhysicsSystem.h"
#include "NewRenderSystem.h"
#include "SDLInputSystem.h"
#include "TimerSystem.h"
#include "SpriteAnimationSystem.h"
#include "GameLoopSystem.h"
#include "AudioSystem.h"
#include "SerializationSystem.h"

#if IMGUI_ENABLED
#include "GuiSystem.h"
#endif

using CoreSystemTypeList = TypeList<
	CameraSystem, 
	PhysicsSystem, 
	SDLInputSystem, 
	TimerSystem,  
	SpriteAnimationSystem,
	GameLoopSystem,
	AudioSystem,
	NewRenderSystem,
	SerializationSystem
>;

#if IMGUI_ENABLED
using GuiSystemTypeList = TypeList<GuiSystem>;
#else
using GuiSystemTypeList = TypeList<>;
#endif

using SystemTypeList = concat_type_lists_t<CoreSystemTypeList, GuiSystemTypeList>;

static_assert(unique_type_list_v<SystemTypeList>);