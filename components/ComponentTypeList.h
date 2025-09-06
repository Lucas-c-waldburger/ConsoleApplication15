#pragma once
#include "../core/TypeUtils.h"

struct ActiveState;
struct MouseState;
struct Transform;
struct RigidBody;
struct Parent;
struct Children;
struct Tags;
struct Collider;
struct GameControllerState;
struct Script;
struct EventCallbacks;
struct CameraTarget;
struct SpriteAnimations;
struct Renderable;
struct EventProductionFlags;
struct GameControllerInputCallbacks;
struct Timer;
struct NeedsUpdate;
struct EntityStateComponent;
struct EntityFlags;
struct SignalTokenStorage;

using ComponentTypeList = TypeList<
	ActiveState,
	MouseState,
	Transform,
	RigidBody,
	Parent,
	Children,
	Tags,
	Collider,
	GameControllerState,
	Script,
	EventCallbacks,
	CameraTarget,
	SpriteAnimations,
	Renderable,
	EventProductionFlags,
	GameControllerInputCallbacks,
	Timer,
	NeedsUpdate,
	EntityStateComponent,
	EntityFlags,
	SignalTokenStorage
>;