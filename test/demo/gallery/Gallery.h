#pragma once
#include "../../../ecs/Ecs.h"
#include "../../../camera/Camera.h"
#include "../../../atlas/NewTextureRepository.h"

namespace test {

class Gallery
{
public:
	struct CameraState
	{
		float lastZoom = 1.0f;
		float lastRotation = 0.0f;
		Camera* camera = nullptr;
	};
	struct CameraOverlay
	{
		Entity zoomTextEntity;
		Entity rotationTextEntity;
	};

	Result<Void> Init(Camera& cam, TextureRepository& repo, SDL_Renderer* renderer, 
					  EventBus& bus, std::string_view pictureDir);

	void Update(float);

private:
	Result<Void> LoadGalleryPictures(TextureRepository& repo, SDL_Renderer* renderer, 
									 std::string_view pictureDir);
	Result<Void> LoadGalleryText(const Handle<TextureResource>& fontHandle,
								 std::vector<std::string>&& words);

	Entity camEntity_;
	std::vector<Entity> pictureEntities_;
	std::vector<Entity> textEntities_;
	CameraOverlay overlayEntities_;
	CameraState cameraState_;
};

} // test