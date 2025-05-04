#pragma once
#pragma warning(disable: 4067) // will bitch at us for no newline eof

#include "ActiveStateComponent.h";
#include "CameraTargetComponent.h"
#include "ColliderComponent.h"
#include "EventObserverComponent.h"
#include "GameControllerStateComponent.h"
#include "RigidBodyComponent.h"
#include "RelationComponents.h"
#include "RenderableComponent.h"
#include "ScriptComponent.h"
#include "MouseStateComponent.h"
#include "TagsComponent.h"
#include "TransformComponent.h"

#define COMPONENT_REGISTRY \
    MouseState, \
    Transform, \
    RigidBody, \
    Parent, \
    Children, \
    Tags, \
    Renderable, \
    Collider, \
    GameControllerState, \
    Script, \
    EventObserver, \
    CameraTarget


