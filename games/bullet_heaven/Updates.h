#pragma once
#include "Components.h"
#include "../../ecs/Ecs.h"

namespace game {

//inline void UpdateBodyStats()
//{
//	ECS::ForAllEntitiesWith<MovementSpeed, RigidBody>([](auto& speed, auto& body){
//		body.limits.linearVelocity.max.x = speed.base * speed.scale;
//	});
//	ECS::ForAllEntitiesWith<JumpHeight, RigidBody>([](auto& height, auto& body) {
//		body.limits.linearVelocity.max.y = height.base * height.scale;
//	});
//}

} // game