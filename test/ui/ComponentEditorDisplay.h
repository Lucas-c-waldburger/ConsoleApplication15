#pragma once
#include "DataEditDisplayUtils.h"
#include "../../ecs/Ecs.h"
//#include "../../serial/user_types/ComponentJsonUserType.h"

class EntityMouseInteractionContext
{
public:
	static inline std::optional<Entity_t> selectedEntity;

private:
	EntityMouseInteractionContext() = default;
};


class EntityMouseInteraction
{
public:
	static bool Update(Entity& e)
	{


		if (ImGui::IsMouseClicked())
	}


private:
	static bool EntityShouldBeSelected(const Entity& e)
	{
		if (!(e.HasComponent<SpriteRenderableComponent>() ||
			  e.HasComponent<TextRenderableComponent>()))
		{
			return false;
		}

		if (!e.HasComponent<Transform>())
		{
			return false;
		}

		const auto& tf = e.GetComponent<Transform>();

		auto mousePos = ImGui::GetMousePos();
	}

	EntityMouseInteraction() = default;
};