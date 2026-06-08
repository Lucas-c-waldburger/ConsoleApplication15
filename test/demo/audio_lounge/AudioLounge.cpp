#include "AudioLounge.h"
#include "../../../file/FilePathUtility.h"
#include "../../../ecs/EntityEvents.h"
#include "../../../inputs/controller/GameController.h"
#include <filesystem>
#include <ranges>

namespace test {

static constexpr float kCdSpriteSize = 256.0f;
static constexpr float kCdTextYOffset = 190.0f;
static constexpr float kCdSpriteAnimChangeTime = 0.08f;
static constexpr float kCdSelectNextMoveTime = 0.9f;
static constexpr float kCdAllowMovePosBuffer = kCdSpriteSize / 3.0f;

namespace {

struct AudioDataPair
{
	std::string name;
	Handle<Audio> handle;
};

Result<Void> LoadCDSprites(TextureRepository& repo)
{
	TRY(ResourcePaths::SpriteDirectory("cd_spin"), cdSpritePaths);

	SpriteDescriptors descriptors{ .seriesName = "cd_spin" };
	descriptors.data = std::move(cdSpritePaths) | std::views::transform([](auto&& path) {
		return SpriteDescriptor{ .filepath = std::move(path) };
	}) | std::ranges::to<std::vector>();

	auto& spriteAtlas = repo.GetSpriteAtlas();

	TRY(spriteAtlas.LoadSprites(SDLite::Renderer(), std::move(descriptors)));

	return kVoid;
}

Result<std::vector<AudioDataPair>> LoadAudio(const std::string& audioDir, AudioBank& bank)
{
	std::vector<AudioDescriptor> descriptors;
	std::vector<AudioDataPair> resultData;

	if (!std::filesystem::exists(audioDir) || !std::filesystem::is_directory(audioDir))
	{
		return MAKE_ERROR("audio directory does not exist or is not a directory");
	}

	for (const auto& entry : std::filesystem::directory_iterator(audioDir))
	{
		if (entry.is_regular_file())
		{
			const auto& path = entry.path();
			const auto ext = path.extension().string();

			if (ext != ".mp3" && ext != ".ogg" && ext != ".flac" && ext != ".wav")
			{
				continue;
			}

			descriptors.emplace_back(AudioDescriptor{ 
				.audioType = AudioType::Music,
				.filepath = path.string() 
			});

			resultData.emplace_back(AudioDataPair{
				.name = path.stem().string()
			});
		}
	}

	if (descriptors.empty())
	{
		return MAKE_ERROR_FMT("No audio files inside found in directory '{}'",
			audioDir);
	}

	for (size_t i = 0; i < descriptors.size(); ++i)
	{
		TRY_ASSIGN(resultData[i].handle, bank.LoadAudio(std::move(descriptors[i])));
		
		assert(resultData[i].handle.IsValid());
	}

	return resultData;
}

Result<Entity> MakeControllerEntity(EventBus& bus)
{
	auto e = ECS::CreateEntity();
	e.AddComponent(CarouselRequest{ .type = CarouselRequest::Type::None });
	e.AddComponent(GameControllerState{});

	auto evs = e.GetEvents(bus);

	TRY(evs.OnEvent([](const events::GameControllerConnected& ev,
		GameControllerState& gcState) {
			if (gcState.joystickID == GameController::kInvalidJoystickID)
			{
				gcState.joystickID = ev.joystickID;
			}
		}));
	TRY(evs.OnEvent([](const events::GameControllerDisconnected& ev,
		GameControllerState& gcState) {
			if (gcState.joystickID == ev.joystickID)
			{
				gcState.joystickID = GameController::kInvalidJoystickID;
			}
		}));
	TRY(evs.OnInput(GameControllerInputSource::LeftStickAxis, (InputState::Pressed | InputState::Held),
		[](const events::GameControllerInput& ev, Entity& e)
		{
			auto& reqType = e.AddComponent<CarouselRequest>().type;

			reqType = (ev.input.value.axis.x < 0)
				? CarouselRequest::Type::MoveLeft
				: CarouselRequest::Type::MoveRight;
		}));

	return e;
}

} // unnamed

void CDCarousel::Update(float dt)
{
	dt = std::min(dt, 1.0f / 30.0f);

	const float targetPosition = static_cast<float>(currentSelection);

	const float displacement = targetPosition - position;

	const float acceleration = displacement * stiffness - velocity * damping;

	velocity += acceleration * dt;
	position += velocity * dt;

	if (std::abs(velocity) < 0.01f && std::abs(position - targetPosition) < 0.01f)
	{
		position = targetPosition;
		velocity = 0.0f;
	}
}

void CDCarousel::Apply(std::vector<Entity>& cdSpriteEntities, std::vector<Entity>& trackNameEntities)
{
	assert(!cdSpriteEntities.empty());
	assert(cdSpriteEntities.size() == trackNameEntities.size());
	assert(static_cast<size_t>(maxSelection) == cdSpriteEntities.size() - 1);

	auto [winCenterX, winCenterY] = SDLite::Window().GetLocalCenter<SDL_FPoint>();

	for (int i = 0; i < cdSpriteEntities.size(); ++i)
	{
		const float offset = (static_cast<float>(i) - position) * spacing;

		const float normalized = std::min(std::abs(offset) / 600.0f, 1.0f);
		//const float scale = std::lerp(1.0f, 0.8f, normalized);

		const float x = winCenterX + offset;
		const float y = winCenterY + std::lerp(0.0f, 20.0f, normalized);

		const float scale = 1.0f - normalized * 0.2f;
		const float alpha = (1.0f - normalized * 0.5f) * 255.0f;

		auto& spriteE = cdSpriteEntities[static_cast<size_t>(i)];
		auto& txtE = trackNameEntities[static_cast<size_t>(i)];

		assert((spriteE.HasComponents<Transform, SpriteRenderableComponent>()));
		assert((txtE.HasComponents<Transform, TextRenderableComponent>()));

		auto& spriteTf = spriteE.GetComponent<Transform>();
		spriteTf.position = { x, y };
		spriteTf.scale = { scale, scale };

		auto& txtTf = txtE.GetComponent<Transform>();
		txtTf.position = { x, y - kCdTextYOffset };
		txtTf.scale = { scale, scale };
		
		auto& spriteProfile = spriteE.GetComponent<SpriteRenderableComponent>().profile;
		spriteProfile.mods.alpha = static_cast<uint8_t>(std::clamp(alpha, 0.0f, 255.0f));

		auto& txtProfile = txtE.GetComponent<TextRenderableComponent>().profile;
		txtProfile.mods.alpha = static_cast<uint8_t>(std::clamp(alpha, 0.0f, 255.0f));
	}
}

Result<Void> AudioLounge2::Init(const std::string& audioDir, AudioBank& audioBank, TextureRepository& repo,
								EventBus& bus)
{
	TRY(LoadAudio(audioDir, audioBank), audioDatas);
	TRY(LoadCDSprites(repo));

	TRY(ResourcePath::Font("GoNotoKurrent-Regular.ttf"), fontPath);
	TRY(repo.GetFontAtlas().LoadFont(SDLite::Renderer(), { .filepath = std::move(fontPath), .fontSize = 28 }),
		fontHandle);

	for (auto&& data : audioDatas)
	{
		assert(data.handle.IsValid());
		audioHandles_.emplace_back(data.handle);

		auto& spriteE = cdSpriteEntities_.emplace_back() = ECS::CreateEntity();
		auto& textE = trackNameEntities_.emplace_back() = ECS::CreateEntity();

		spriteE.AddComponent(Transform{});
		spriteE.AddComponent(SpriteRenderableComponent{});
		spriteE.AddComponent(SpriteAnimationComponent{ .spriteSeriesName = "cd_spin" });

		textE.AddComponent(Transform{});
		textE.AddComponent(TextRenderableComponent{
			.writer = {.resourceHandle = fontHandle, .text = std::move(data.name) },
			.formatting = {.bounds = { 1, 1 }, .align = TextAlign::Center, .scaleToBounds = false},
			.profile = {.mods = {.color = RGB::FromSDLColor(SDLite::kColorWhite) }}
		});
	}

	assert(!cdSpriteEntities_.empty());
	carousel_.maxSelection = cdSpriteEntities_.size() - 1;

	GetActiveSpriteEntity().AddComponent(NewAudioRequest{
		.audioHandle = audioHandles_[static_cast<size_t>(carousel_.currentSelection)],
		.settings = {
			.fadeMs = AudioFadeMs{ .in = 100, .out = 100 }
		}
	});

	[[maybe_unused]] const bool registered = ECS::RegisterComponent<CarouselRequest>();
	assert(registered);

	TRY_ASSIGN(controllerEntity_, MakeControllerEntity(bus));

	return kVoid;
}

void AudioLounge2::Update(float dt)
{
	assert(controllerEntity_.HasComponent<CarouselRequest>());

	auto& reqType = controllerEntity_.GetComponent<CarouselRequest>().type;

	auto activeSpriteX = GetActiveSpriteEntity().GetComponent<Transform>().position.x;
	const auto winCenterX = SDLite::Window().GetLocalCenter<SDL_FPoint>().x;

	if (std::abs(winCenterX - activeSpriteX) > kCdAllowMovePosBuffer)
	{
		reqType = CarouselRequest::Type::None;
	}
	else if (reqType != CarouselRequest::Type::None)
	{
		auto& oldActiveSpriteE = GetActiveSpriteEntity();

		const bool moved = (reqType == CarouselRequest::Type::MoveLeft) 
			? carousel_.MoveLeft() 
			: carousel_.MoveRight();

		if (moved)
		{
			oldActiveSpriteE.GetComponent<SpriteAnimationComponent>().index.current = 0;

			if (oldActiveSpriteE.HasComponent<ActiveAudio>())
			{
				//const auto audioInst = oldActiveSpriteE.GetComponent<ActiveAudio>().instanceId;
				//assert(audioInst.IsValid());

				oldActiveSpriteE.AddComponent(AudioUpdateRequest{
					//.instanceId = audioInst,
					.command = AudioPlayCommand::Stop
				});
			}

			auto& newActiveSpriteE = GetActiveSpriteEntity();
			newActiveSpriteE.AddComponent(NewAudioRequest{
				.audioHandle = audioHandles_[static_cast<size_t>(carousel_.currentSelection)],
				.settings = {
					.fadeMs = AudioFadeMs{.in = 100, .out = 100 }
				}
			});
		}
	}

	carousel_.Update(dt);
	timeInCurrentCdSpriteFrame_ += dt;

	auto& activeSpriteE = GetActiveSpriteEntity();
	activeSpriteX = activeSpriteE.GetComponent<Transform>().position.x;

	if (timeInCurrentCdSpriteFrame_ >= kCdSpriteAnimChangeTime)
	{
		timeInCurrentCdSpriteFrame_ = 0.0f;

		if (std::abs(winCenterX - activeSpriteX) < kCdAllowMovePosBuffer)
		{
			assert(activeSpriteE.HasComponent<SpriteAnimationComponent>());

			++activeSpriteE.GetComponent<SpriteAnimationComponent>().index;
		}
	}

	carousel_.Apply(cdSpriteEntities_, trackNameEntities_);
}

Entity& AudioLounge2::GetActiveSpriteEntity()
{
	assert(carousel_.currentSelection < cdSpriteEntities_.size());

	return cdSpriteEntities_[static_cast<size_t>(carousel_.currentSelection)];
}

//SDL_FPoint AudioLounge2::GetTargetCDPosition() const
//{
//	assert(carousel_.currentSelection < cdSpriteEntities_.size());
//
//	auto& targetE = cdSpriteEntities_[static_cast<size_t>(carousel_.currentSelection)];
//	assert(targetE.HasComponent<Transform>());
//
//	return targetE.GetComponent<Transform>().position;
//}

void AudioLounge::CD::SetPosition(SDL_FPoint pos)
{
	auto setPos = [](Entity& e, SDL_FPoint p) {
		assert(e.IsValid() && e.HasComponent<Transform>());
		e.GetComponent<Transform>().position = p;
	};

	setPos(cdSpriteEntity, pos);
	setPos(trackNameEntity, { pos.x, pos.y - kCdTextYOffset });
}

void AudioLounge::CD::SetVisible(bool tf)
{
	cdSpriteEntity.SetComponentVisibility<SpriteRenderableComponent>(tf);
	trackNameEntity.SetComponentVisibility<TextRenderableComponent>(tf);
}

void AudioLounge::CD::SetAlpha(uint8_t alpha)
{
	cdSpriteEntity.GetComponent<SpriteRenderableComponent>().profile.mods.alpha = alpha;
	trackNameEntity.GetComponent<TextRenderableComponent>().profile.mods.alpha = alpha;
}

auto AudioLounge::MakeCDSpritePositions() -> CDSpritePositions
{
	auto winW = SDLite::Window().GetSize<float>().w;
	auto winCenter = SDLite::Window().GetLocalCenter<SDL_FPoint>();

	return {
		.left = { 0, winCenter.y },
		.center = winCenter,
		.right = { winW, winCenter.y }
	};
}

Result<Void> AudioLounge::Init(const std::string& audioDir, AudioBank& audioBank, TextureRepository& repo)
{
	cdSpritePositions_ = MakeCDSpritePositions();

	TRY(LoadAudio(audioDir, audioBank), audioDatas);
	TRY(LoadCDSprites(repo));

	TRY(ResourcePath::Font("GoNotoKurrent-Regular.ttf"), fontPath);
	TRY(repo.GetFontAtlas().LoadFont(SDLite::Renderer(), {.filepath = std::move(fontPath), .fontSize = 28}),
		fontHandle);

	SDL_FPoint currentCdPos = SDLite::Window().GetLocalCenter<SDL_FPoint>();
	for (size_t i = 0; i < audioDatas.size(); ++i)
	{
		auto& data = audioDatas[i];

		auto& cd = cds_.emplace_back();

		cd.audioHandle = data.handle;

		auto& spriteE = cd.cdSpriteEntity = ECS::CreateEntity();

		spriteE.AddComponent(Transform{ .position = currentCdPos });
		spriteE.AddComponent(SpriteRenderableComponent{
			.profile = { .mods = { .alpha = (i > 0 ? 128 : 255 )}}
		});
		spriteE.AddComponent(SpriteAnimationComponent{ .spriteSeriesName = "cd_spin" });

		//spriteE.SetComponentVisibility<SpriteRenderableComponent>(false);

		auto& txtE = cd.trackNameEntity = ECS::CreateEntity();

		txtE.AddComponent(Transform{ .position = { currentCdPos.x, currentCdPos.y - kCdTextYOffset } });
		txtE.AddComponent(TextRenderableComponent{
			.writer = { .resourceHandle = fontHandle, .text = std::move(data.name) },
			.formatting = { .bounds = { 1, 1 }, .align = TextAlign::Center, .scaleToBounds = false},
			.profile = { .mods = { .color = RGB::FromSDLColor(SDLite::kColorWhite) }}
		});

		//txtE.SetComponentVisibility<TextRenderableComponent>(i == 0);

		currentCdPos.x += kCdSpritesXSpacing;
	}

	//assert(!cds_.empty());
	//cds_[0].SetPosition(cdSpritePositions_.center);
	//cds_[0].SetVisible(true);
	//cds_[0].SetAlpha(255);

	//if (cds_.size() > 1)
	//{
	//	cds_[1].SetPosition(cdSpritePositions_.right);
	//	cds_[1].SetVisible(true);
	//	cds_[1].SetAlpha(126);
	//}

	return kVoid;
}

void AudioLounge::Update(float dt)
{
	assert(currentCdIndex_ < cds_.size());

	//if (goNextCd_ != GoNextCD::None)
	//{

	//}

	timeInCurrentCdSpriteFrame += dt;
	if (timeInCurrentCdSpriteFrame >= kCdSpriteAnimChangeTime)
	{
		++cds_[currentCdIndex_].cdSpriteEntity.GetComponent<SpriteAnimationComponent>().index;
		timeInCurrentCdSpriteFrame = 0.0f;
	}
}





} // test