#include "DelegateCleanupSystem.h"
#include "../ecs/Ecs.h"

void DelegateCleanupSystem::Update()
{
	// find delegates with all delegated components removed
	auto delegates = ECS::GetAllEntitiesWithOnly<ComponentDelegate>();

	for (auto& delegate : delegates)
	{
		delegate.Destroy();
	}
}
