#pragma once
#include "ActiveStateComponent.h";
#include "ColliderComponent.h"
#include "ForceAccumulatorComponent.h"
#include "GameControllerStateComponent.h"
#include "PhysicsComponent.h"
#include "RelationComponents.h"
#include "RenderableComponent.h"
#include "ScriptComponent.h"
#include "SpatialComponent.h"
#include "TagsComponent.h"
#include "TransformComponent.h"

#define COMPONENT_REGISTRY \
    Spatial, \
    Transform, \
    Physics, \
    Parent, \
    Children, \
    Tags, \
    Renderable, \
    Collider, \
    GameControllerState, \
    ForceAccumulator, \
    Script 