#include "TallySystem.h"
#include "../ecs/Ecs.h"




//void TallySystem::Update(double deltaTime)
//{
//	auto entities = ECS::GetAllEntitiesWith<EntityMetrics>();
//
//	for (auto& entity : entities)
//	{
//		auto& metrics = entity.GetComponent<EntityMetrics>();
//
//		metrics.activeTime.total += deltaTime;
//
//		if (metrics.travelDistance.has_value())
//		{
//			if (entity.HasComponent<Transform>())
//			{
//				SDL_FPoint currentPos = entity.GetComponent<Transform>().position;
//
//				metrics.travelDistance->tr
//
//			}
//			else
//			{
//				LOG_WARNING("Entity had travel distance metric "
//					"but did not have transform component");
//			}
//		}
//	}
//}
