#pragma once
#include "../scripting/LuaTypesRegistry.h"
#include "../ecs/Ecs.h"


Result<Void> HookPhysicsTestScript(Entity& entity)
{
	if (!entity.HasComponent<Physics>())
	{
		return MAKE_ERROR("Entity did not have a physics component");
	}
	auto& physics = entity.GetComponent<Physics>();



}