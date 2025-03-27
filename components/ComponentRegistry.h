#pragma once
#include "BaseComponent.h"

struct Spatial;
struct Transform;
struct Physics;
struct Parent;
struct Children;
struct Tags;
struct Renderable;
struct Collider;
struct GameControllerState;
struct ForceAccumulator;

using ComponentRegistry = TypeList<
    Spatial,
    Transform,
    Physics,
    Parent,
    Children,
    Tags,
    Renderable,
    Collider,
    GameControllerState
>;