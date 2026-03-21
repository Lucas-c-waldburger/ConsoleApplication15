#pragma once
#include "../../Fixtures.h"
#include "../../../ecs/Ecs.h"
#include "../../../ecs/EntityEvents.h"
#include "../../../events/data/EventDataIncludes.h"
#include "../../../file/FilePathUtility.h"
#include "../../../inputs/controller/GameController.h"


namespace test {

static constexpr std::string_view kThumbstickOuterSpritePath = 
	"ui/joystick/Joystick.png";
static constexpr std::string_view kThumbstickInnerSpritePath = 
	"ui/joystick/LargeHandleFilledGrey.png";
static constexpr std::string_view kControllerButtonSpriteAPath = "ui/controller_btn/btn_a.png";
static constexpr std::string_view kControllerButtonSpriteBPath = "ui/controller_btn/btn_b.png";
static constexpr std::string_view kControllerButtonSpriteXPath = "ui/controller_btn/btn_x.png";
static constexpr std::string_view kControllerButtonSpriteYPath = "ui/controller_btn/btn_y.png";

static constexpr Dimensions<float> kOuterThumbstickSpriteDimensions = { 300.0f, 300.0f };
static constexpr Dimensions<float> kInnerThumbstickSpriteDimensions = { 140.0f, 140.0f };
static constexpr float kOuterThumbstickSpriteScale = 0.45f;
static constexpr float kInnerThumbstickSpriteScale = 0.65f;
static constexpr float kMaxInnerDeltaMovePx = 65.0f * kInnerThumbstickSpriteScale;
static constexpr float kDefaultPlacementPadding = 60.0f;

static constexpr float kControllerButtonSpritePosOffset = 50.0f;
static constexpr float kControllerButtonSpriteScale = 1.0f;
static constexpr uint8_t kControllerButtonTransparentAlpha = 100;

class ThumbstickUiDraw
{
public:
	ThumbstickUiDraw() = default;

	void Update(float)
	{
		auto girl = ECS::GetEntityByID(girlId_);
		if (!girl.IsValid() || !girl.HasComponent<GameControllerState>())
		{
			return;
		}

		auto inE = ECS::GetEntityByID(innerE_);
		if (!inE.IsValid() || !inE.HasComponent<Transform>())
		{
			return;
		}

		auto& tf = inE.GetComponent<Transform>();

		const auto& gc = girl.GetComponent<GameControllerState>();
		if (gc.joystickID == -1)
		{
			return;
		}

		const auto& stick = gc.inputs[GameControllerInputSource::LeftStickAxis];

		//if (stick.state == InputState::Released ||
		//	stick.state == InputState::None)
		//{
		//	tf.position = centerPos_;
		//	return;
		//}

		const int axisValX = stick.value.axis.x;
		const int axisValY = stick.value.axis.y;
		if (std::abs(axisValX) < 2600 && std::abs(axisValY) < 2600)
		{
			tf.position = centerPos_;
			return;
		}

		const float normedX = static_cast<float>(axisValX) /
			static_cast<float>(GameController::kAxisMax);
		const float normedY = static_cast<float>(axisValY) /
			static_cast<float>(GameController::kAxisMax);

		const float posX = centerPos_.x + (normedX * kMaxInnerDeltaMovePx);
		const float posY = centerPos_.y + (normedY * kMaxInnerDeltaMovePx);

		tf.position = { posX, posY };
	}

	static Result<Void> CreateAndConnect(const Entity& girlE, 
		SceneFixture::SharedPtr& fixture, 
		std::optional<SDL_FPoint> pos = std::nullopt)
	{
		TRY(ResourcePath::Sprite(kThumbstickOuterSpritePath), outerPath);
		TRY(ResourcePath::Sprite(kThumbstickInnerSpritePath), innerPath);

		auto& atlas = fixture->GetTextureRepository().GetSpriteAtlas();
		TRY(atlas.LoadSprite(fixture->GetRenderer(), { .filepath = std::move(outerPath) }),
			outerSprite);
		TRY(atlas.LoadSprite(fixture->GetRenderer(), { .filepath = std::move(innerPath) }),
			innerSprite);
		
		auto outE = ECS::CreateEntity();
		auto inE = ECS::CreateEntity();
		assert(outE.IsValid());
		assert(inE.IsValid());

		if (!pos.has_value())
		{
			const float winH = SDLite::Window().GetSize<float>().h;
			const float spriteSize = kOuterThumbstickSpriteDimensions.w *
									 kOuterThumbstickSpriteScale;
			const float spriteCenter = spriteSize / 2.0f;

			pos = SDL_FPoint{ 
				spriteCenter + kDefaultPlacementPadding, 
				winH - spriteCenter - kDefaultPlacementPadding
			};
		}

		outE.AddComponent(Transform{ 
			.position = *pos,
			.scale = { kOuterThumbstickSpriteScale, kOuterThumbstickSpriteScale }
		});
		inE.AddComponent(Transform{ 
			.position = *pos,
			.scale = { kInnerThumbstickSpriteScale, kInnerThumbstickSpriteScale }
		});

		outE.AddComponent(SpriteRenderableComponent{ 
			.sprite = outerSprite,
			.profile = { .drawOrder = 1000, .isOverlay = false }
		});
		inE.AddComponent(SpriteRenderableComponent{
			.sprite = innerSprite,
			.profile = { .drawOrder = 1001, .isOverlay = false }
		});

		//TRY(SetUpEvents(inE, fixture->GetEventBus(), *pos));

		fixture->RegisterSystem<ThumbstickUiDraw>(Phase::Input, girlE, outE, inE, *pos);

		return kVoid;
	}
	
	void Destroy()
	{
		auto outE = ECS::GetEntityByID(outerE_);
		if (outE.IsValid()) { outE.Destroy(); }

		auto inE = ECS::GetEntityByID(innerE_);
		if (inE.IsValid()) { inE.Destroy(); }
	}


	ThumbstickUiDraw(const Entity& girlE, Entity& outE, Entity& inE, SDL_FPoint pos) : 
		girlId_(girlE.GetID()), outerE_(outE.GetID()), innerE_(inE.GetID()),
		centerPos_(pos) {}

private:
	Entity_t girlId_ = kInvalidEntity;
	Entity_t outerE_ = kInvalidEntity;
	Entity_t innerE_ = kInvalidEntity;
	SDL_FPoint centerPos_ = { 0.0f, 0.0f };
};

class ControllerButtonUiDraw
{
public:
	using ButtonEntityMap = std::unordered_map<GameControllerInputSource, Entity_t>;

	explicit ControllerButtonUiDraw(const Entity& girl, ButtonEntityMap&& map) : 
		girlId_(girl.GetID()), buttonEntityIds_(std::move(map)) {}

	void Update(float)
	{
		auto girl = ECS::GetEntityByID(girlId_);
		if (!girl.IsValid() || !girl.HasComponent<GameControllerState>())
		{
			return;
		}

		const auto& gc = girl.GetComponent<GameControllerState>();
		if (gc.joystickID == -1)
		{
			return;
		}

		for (const auto& [btn, entId] : buttonEntityIds_)
		{
			auto btnE = ECS::GetEntityByID(entId);
			if (!btnE.IsValid() || !btnE.HasComponent<SpriteRenderableComponent>())
			{
				continue;
			}

			auto& rend = btnE.GetComponent<SpriteRenderableComponent>();

			const auto& field = gc.inputs[btn];
			switch (field.state)
			{
			case InputState::Pressed:
			case InputState::Held:
				rend.profile.mods.alpha = 255;
				break;
			default:
				rend.profile.mods.alpha = kControllerButtonTransparentAlpha;
				break;
			}
		}
	}

	static Result<Void> CreateAndConnect(const Entity& girlE,
		SceneFixture::SharedPtr& fixture,
		std::optional<SDL_FPoint> pos = std::nullopt)
	{
		TRY(ResourcePath::Sprite(kControllerButtonSpriteAPath), aPath);
		TRY(ResourcePath::Sprite(kControllerButtonSpriteBPath), bPath);
		TRY(ResourcePath::Sprite(kControllerButtonSpriteXPath), xPath);
		TRY(ResourcePath::Sprite(kControllerButtonSpriteYPath), yPath);

		auto& atlas = fixture->GetTextureRepository().GetSpriteAtlas();
		TRY(atlas.LoadSprite(fixture->GetRenderer(), {.filepath = std::move(aPath)}), aSprite);
		TRY(atlas.LoadSprite(fixture->GetRenderer(), {.filepath = std::move(bPath)}), bSprite);
		TRY(atlas.LoadSprite(fixture->GetRenderer(), {.filepath = std::move(xPath)}), xSprite);
		TRY(atlas.LoadSprite(fixture->GetRenderer(), {.filepath = std::move(yPath)}), ySprite);

		if (!pos.has_value())
		{
			const float winH = SDLite::Window().GetSize<float>().h;
			const float spriteSize = kOuterThumbstickSpriteDimensions.w *
				kOuterThumbstickSpriteScale;
			const float spriteCenter = spriteSize / 2.0f;

			pos = SDL_FPoint{
				spriteCenter + kDefaultPlacementPadding,
				winH - ((spriteCenter + (kDefaultPlacementPadding * 1.4f)) * 2.0f)
			};
		}

		ButtonEntityMap map;

		auto makeEnt = [&map, pos = *pos](auto btnEnum, auto&& sprite, float offX, float offY) {
			auto e = ECS::CreateEntity();
			assert(e.IsValid());

			e.AddComponent(Transform{
				.position = { pos.x + offX, pos.y + offY },
				.scale = { kControllerButtonSpriteScale, kControllerButtonSpriteScale }
			});
			e.AddComponent(SpriteRenderableComponent{
				.sprite = std::move(sprite),
				.profile = { 
					.drawOrder = 1000, 
					.mods = { .alpha = kControllerButtonTransparentAlpha },
					.isOverlay = true
				}
			});

			map[btnEnum] = e.GetID();
		};

		using enum GameControllerInputSource;

		makeEnt(A, std::move(aSprite), 0.0f, kControllerButtonSpritePosOffset);
		makeEnt(B, std::move(bSprite), kControllerButtonSpritePosOffset, 0.0f);
		makeEnt(X, std::move(xSprite), -kControllerButtonSpritePosOffset, 0.0f);
		makeEnt(Y, std::move(ySprite), 0.0f, -kControllerButtonSpritePosOffset);

		fixture->RegisterSystem<ControllerButtonUiDraw>(
			Phase::Input, girlE, std::move(map));

		return kVoid;
	}

private:
	Entity_t girlId_ = kInvalidEntity;
	ButtonEntityMap buttonEntityIds_;
};












} // 