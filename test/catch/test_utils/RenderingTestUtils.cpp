#include "RenderingTestUtils.h"
#include "../../../atlas/TextureRepository.h"
#include "../../../ecs/Ecs.h"
#include "../../../sdl/SDLite.h"

Result<Void> test::QuickRender(const Entity& entity, const NewTextureRepository& textureRepo)
{
	//if (!SDLite::Running())
	//{
	//	return MAKE_ERROR("SDL was not initialized");
	//}
	//if (!entity.IsValid())
	//{
	//	return MAKE_ERROR("Entity was invalid");
	//}
	//if (!entity.HasComponent<NewRenderable>())
	//{
	//	return MAKE_ERROR("Entity does not have a renderable component");
	//}
	//if (!entity.HasComponent<Transform>())
	//{
	//	return MAKE_ERROR("Entity does not have a transform component");
	//}

	//auto& renderable = entity.GetComponent<NewRenderable>();

	//if (std::holds_alternative<NewTextRenderable>(renderable.renderData))
	//{
	//	if (!entity.HasComponent<)
	//}
	return Void{};
}
