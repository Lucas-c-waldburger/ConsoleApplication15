#pragma once
#include "../core/TypeUtils.h"

struct ActiveState;
struct MouseState;
struct Transform;
struct RigidBody;
struct Parent;
struct Children;
struct Tags;
struct Renderable;
struct Collider;
struct GameControllerState;
struct Script;
struct EventCallbacks;
struct CameraTarget;
struct SpriteAnimations;
struct NewRenderable;
struct EventProductionFlags;


using ComponentTypeList = TypeList<
	ActiveState,
	MouseState,
	Transform,
	RigidBody,
	Parent,
	Children,
	Tags,
	Renderable,
	Collider,
	GameControllerState,
	Script,
	EventCallbacks,
	CameraTarget,
	SpriteAnimations,
	NewRenderable,
	EventProductionFlags
>;