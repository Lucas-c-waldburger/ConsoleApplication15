#include "ComponentCleanup.h"
#include "../components/RigidBodyComponent.h"
#include "../components/ColliderComponent.h"
#include "Ecs.h"
#include <ranges>


void CleanupComponent(Entity&& e, RigidBody& rb)
{	
	WriteAccessor<B2Body> bodyAccess{};
	WriteAccessor<B2Shape> shapeAccess{};

	auto& body = bodyAccess(rb.body);

	static constexpr auto hasValidShape = [](const Collider& col) {
		return col.shape.GetData().IsValid();
	};

	if (e.HasComponent<Collider>(hasValidShape))
	{
		auto& shape = shapeAccess(e.GetComponent<Collider>().shape);
		if (shape.GetParentBodyHandle() == body.GetHandle())
		{
			shape.Destroy();
		}
	}

	if (e.HasComponent<Children>())
	{
		const auto& children = e.GetComponent<Children>().childEntityIds;
		for (auto ch : children | std::views::transform(&ECS::GetEntityByID))
		{
			if (ch.HasComponent<Collider>())
			{
				auto& shape = shapeAccess(ch.GetComponent<Collider>().shape);
				if (shape.GetParentBodyHandle() == body.GetHandle())
				{
					shape.Destroy();
				}
			}
		}
	}
}

void CleanupComponent(Entity&&, Collider& col)
{
	WriteAccessor<B2Shape>{}(col.shape).Destroy();
}