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
struct CameraTarget;
struct Timer;
struct NeedsUpdate;
struct EntityStateComponent;
struct EntityFlags;
struct SignalTokenStorage;
struct NewAudioRequest;
struct AudioUpdateRequest;
struct ActiveAudio;
struct TextRenderableGlyphCache;
struct TextRenderableComponent;
struct SpriteRenderableComponent;
struct MarkedDestroyed;
struct SpriteAnimationComponent;
struct NeedsAnimationUpdate;

using CoreComponentTypeList = TypeList<
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
	CameraTarget,
	Timer,
	NeedsUpdate,
	EntityStateComponent,
	EntityFlags,
	SignalTokenStorage,
	NewAudioRequest,
	AudioUpdateRequest,
	ActiveAudio,
	TextRenderableGlyphCache,
	TextRenderableComponent,
	SpriteRenderableComponent,
	MarkedDestroyed,
	SpriteAnimationComponent,
	NeedsAnimationUpdate
>;