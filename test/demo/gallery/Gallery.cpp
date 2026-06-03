#include "Gallery.h"
#include "../../../ecs/EntityEvents.h"
#include "../../../inputs/controller/GameController.h"
#include "../../../file/FilePathUtility.h"
#include <filesystem>
#include <format>
#include <random>

namespace test {

namespace {

constexpr float kMaxCameraRotationIncrement = 5.0f;
constexpr float kMaxCameraPanIncrement = 15.0f;

constexpr float kPictureSpacing = 10.0f;

constexpr bool PressedOrHeld(const events::GameControllerInput& ev)
{
	return ev.input.state == InputState::Pressed || ev.input.state == InputState::Held;
}

auto MakeRStickRotationCallback(Camera& cam)
{
	return [&cam](const events::GameControllerInput& ev)
	{
		if (!PressedOrHeld(ev))
		{
			return;
		}

		float axisX = static_cast<float>(ev.input.value.axis.x);
		const float axisNorm = axisX / static_cast<float>(GameController::kAxisMax);

		const float rotationIncrement = axisNorm * kMaxCameraRotationIncrement;

		cam.SetRotation(cam.GetRotation() + rotationIncrement);
	};
}

auto MakeTriggerZoomCallback(Camera& cam, bool zoomIn)
{
	return [&cam, zoomIn](const events::GameControllerInput& ev)
	{
		if (!PressedOrHeld(ev))
		{
			return;
		}

		const float triggerNorm = static_cast<float>(ev.input.value.trigger) /
								  static_cast<float>(GameController::kAxisMax);
		const float zoomIncrement = triggerNorm * 0.07f;

		cam.SetZoomScale(std::max(0.1f, cam.GetZoomScale() + (zoomIn ? zoomIncrement : -zoomIncrement)));
	};
}

auto MakeLStickMoveCallback()
{
	return [](const events::GameControllerInput& ev, Transform& tf)
	{
		if (!PressedOrHeld(ev))
		{
			return;
		}

		const float axisNormX = static_cast<float>(ev.input.value.axis.x) /
								static_cast<float>(GameController::kAxisMax);
		const float axisNormY = static_cast<float>(ev.input.value.axis.y) /
								static_cast<float>(GameController::kAxisMax);

		const float panIncrementX = axisNormX * kMaxCameraPanIncrement;
		const float panIncrementY = axisNormY * kMaxCameraPanIncrement;

		tf.position.x += panIncrementX;
		tf.position.y += panIncrementY;
	};
}

} // unnamed

Result<Void> Gallery::Init(Camera& cam, TextureRepository& repo, SDL_Renderer* renderer, 
						   EventBus& bus, std::string_view pictureDir)
{
	assert(renderer);

	camEntity_ = ECS::CreateEntity();

	camEntity_.AddComponent(Transform{ .position = SDLite::Window().GetLocalCenter<SDL_FPoint>() });
	camEntity_.AddComponent(CameraTarget{});
	camEntity_.AddComponent(GameControllerState{});

	auto evs = camEntity_.GetEvents(bus);
	evs.OnEvent([](const events::GameControllerConnected& ev,
				   GameControllerState& gcState) {
		if (gcState.joystickID == GameController::kInvalidJoystickID)
		{
			gcState.joystickID = ev.joystickID;
		}
	});
	evs.OnEvent([](const events::GameControllerDisconnected& ev,
				   GameControllerState& gcState) {
		if (gcState.joystickID == ev.joystickID)
		{
			gcState.joystickID = GameController::kInvalidJoystickID;
		}
	});

	using Src = GameControllerInputSource;
	evs.OnInput(Src::RightStickAxis, MakeRStickRotationCallback(cam));
	evs.OnInput(Src::RightTrigger, MakeTriggerZoomCallback(cam, true));
	evs.OnInput(Src::LeftTrigger, MakeTriggerZoomCallback(cam, false));
	evs.OnInput(Src::LeftStickAxis, MakeLStickMoveCallback());

	//TRY(LoadGalleryPictures(repo, renderer, pictureDir));

	TRY(ResourcePath::Font("GoNotoKurrent-Regular.ttf"), fontPath);

	TRY(repo.GetFontAtlas().LoadFont(renderer, { .filepath = std::move(fontPath), .fontSize = 28 }),
		fontHandle);

	TRY(LoadGalleryText(fontHandle, {
		"lol", "hey bro", "this is a gallery demo", "the quick brown fox jumps over the lazy dog",
		"Gottem", "Skibidi", "Bop", "Yes", "No", "Maybe", "I don't know", "What is this?",
	}));

	cameraState_.camera = &cam;
	auto& zoomE = overlayEntities_.zoomTextEntity = ECS::CreateEntity();
	assert(zoomE.IsValid());
	zoomE.AddComponent(Transform{ .position = { 20.0f, 20.0f } });
	zoomE.AddComponent(TextRenderableComponent{
		.writer = GlyphTextWriter{ .resourceHandle = fontHandle, .text = "Zoom: 1.00" },
		.formatting = {
			.scaleToBounds = false
		},
		.profile = {
			.mods = { .color = { 255, 255, 255 }, .alpha = 255},
			.isOverlay = true
		}
	});

	auto& rotE = overlayEntities_.rotationTextEntity = ECS::CreateEntity();
	assert(rotE.IsValid());
	rotE.AddComponent(Transform{ .position = { 20.0f, 50.0f } });
	rotE.AddComponent(TextRenderableComponent{
		.writer = GlyphTextWriter{ .resourceHandle = fontHandle, .text = "Rotation: 0.00" },
		.formatting = {
			.scaleToBounds = false
		},
		.profile = {
			.mods = { .color = { 255, 255, 255 }, .alpha = 255},
			.isOverlay = true
		}
	});

	return kVoid;
}

void Gallery::Update(float)
{
	if (!cameraState_.camera)
	{
		return;
	}

	const float newZoom = cameraState_.camera->GetZoomScale();
	const float newRot = cameraState_.camera->GetRotation();

	if (newZoom != cameraState_.lastZoom)
	{
		cameraState_.lastZoom = newZoom;
		if (overlayEntities_.zoomTextEntity.IsValid())
		{
			auto& text = 
				overlayEntities_.zoomTextEntity.GetComponent<TextRenderableComponent>().writer.text;
			text = std::format("Zoom: {:.2f}", newZoom);
		}
	}
	if (newRot != cameraState_.lastRotation)
	{
		cameraState_.lastRotation = newRot;
		if (overlayEntities_.rotationTextEntity.IsValid())
		{
			auto& text = 
				overlayEntities_.rotationTextEntity.GetComponent<TextRenderableComponent>().writer.text;
			text = std::format("Rotation: {:.2f}", newRot);
		}
	}
}

Result<Void> Gallery::LoadGalleryPictures(TextureRepository& repo, SDL_Renderer* renderer, 
										  std::string_view pictureDir)
{
	SpriteDescriptors descriptors;

	if (!std::filesystem::exists(pictureDir) || !std::filesystem::is_directory(pictureDir))
	{
		return MAKE_ERROR("Picture directory does not exist or is not a directory");
	}

	for (const auto& entry : std::filesystem::directory_iterator(pictureDir))
	{
		if (entry.is_regular_file())
		{
			const auto& path = entry.path();
			const auto ext = path.extension().string();

			if (ext != ".png" && ext != ".jpg" && ext != ".jpeg")
			{
				continue;
			}

			descriptors.data.emplace_back(SpriteDescriptor{ .filepath = path.string() });
		}
	}

	TRY(repo.GetSpriteAtlas().LoadSprites(renderer, std::move(descriptors)), sprites);
	std::shuffle(sprites.begin(), sprites.end(), std::mt19937{ std::random_device{}() });

	const size_t numRowsCols = std::ceil(std::sqrt(sprites.size()));

	size_t numInCurrentRow = 0;
	float currentX = 0.0f;
	float currentY = 0.0f;
	for (auto&& sprite : sprites)
	{
		if (numInCurrentRow >= numRowsCols)
		{
			numInCurrentRow = 0;
			currentX = 0.0f;
			currentY += sprite.plot.rect.h + kPictureSpacing;
		}

		currentX += (sprite.plot.rect.w / 2.0f) + kPictureSpacing;
		numInCurrentRow++;

		auto& e = pictureEntities_.emplace_back(ECS::CreateEntity());

		e.AddComponent(Transform{
			.position = { currentX, currentY }
		});
		e.AddComponent(SpriteRenderableComponent{ .sprite = std::move(sprite) });
	}

	return kVoid;
}

Result<Void> Gallery::LoadGalleryText(const Handle<TextureResource>& fontHandle,
									  std::vector<std::string>&& words)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> posDistrib(-500, 1000);
	std::uniform_int_distribution<int> clrDistrib(0, 255);

	auto getPos = [&posDistrib, &gen]() {
		return SDL_FPoint{ static_cast<float>(posDistrib(gen)), static_cast<float>(posDistrib(gen)) };
	};

	for (auto&& word : words)
	{
		auto& e = textEntities_.emplace_back(ECS::CreateEntity());
		assert(e.IsValid());

		e.AddComponent(Transform{ .position = getPos() });
		e.AddComponent(TextRenderableComponent{
			.writer = GlyphTextWriter{ .resourceHandle = fontHandle, .text = std::move(word) },
			.formatting = {
				.scaleToBounds = false
			},
			.profile = {
				.mods = { .color = { clrDistrib(gen), clrDistrib(gen), clrDistrib(gen) }, .alpha = 255 }
			}
		});
	}

	return kVoid;
}

} // test