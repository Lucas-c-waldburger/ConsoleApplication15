#pragma once
#include "Components.h"
#include "../../test/Fixtures.h"
#include "../../sdl/SDLUtils.h"
#include "../../ecs/Ecs.h"
#include "../../ecs/EntityEvents.h"
#include "../../events/data/EventDataIncludes.h"
#include "../../inputs/controller/GameControllerUtils.h"

namespace game {

static constexpr std::string_view kSwordSlashDirPath = "slash_effect/Slash 2/color1/Frames";
static constexpr std::string_view kCrosshairPath = "crosshairs/Sight_64x64_001.png";
static constexpr std::string_view kKnightPath = "knight_new/idle/Idle_000.png";

static const std::vector<std::string> GetSwordSlashSpritePaths()
{
	static std::vector<std::string> spritePaths; 
	if (spritePaths.empty())
	{
		auto result = ResourcePaths::SpriteDirectory(kSwordSlashDirPath);
		assert(result.Success());

		spritePaths = std::move(result).GetValue();
	}

	return spritePaths;
}
static const std::string& GetKnightSpritePath()
{
	static std::string knightPath;
	if (knightPath.empty())
	{
		auto result = ResourcePath::Sprite(kKnightPath);
		assert(result.Success());

		knightPath = std::move(result).GetValue();
	}

	return knightPath;
}

static const std::string& GetCrosshairSpritePath()
{
	static std::string crosshairPath;
	if (crosshairPath.empty())
	{
		auto result = ResourcePath::Sprite(kCrosshairPath);
		assert(result.Success());

		crosshairPath = std::move(result).GetValue();
	}

	return crosshairPath;
}

static SDL_FPoint MapAxisToRadius(SDL_Point axis, float radius)
{
	SDL_FPoint normed = {
		static_cast<float>(axis.x) / static_cast<float>(SDL_JOYSTICK_AXIS_MAX),
		static_cast<float>(axis.y) / static_cast<float>(SDL_JOYSTICK_AXIS_MAX)
	};

	float magnitude = std::min(
		std::sqrt(normed.x * normed.x + normed.y * normed.y), 1.0f);

	SDL_FPoint dir = (magnitude > 0.0f)
		? normed / magnitude
		: SDL_FPoint{ 0.0f, 0.0f };

	return { dir * radius * magnitude };
}

struct CrosshairComponent
{
	float radius = 100.0f;
	SDL_FPoint parentOffset = { 0.0f, 0.0f };
	bool active = false;
};



static Result<Entity> SetUpCrosshairEntity(Entity& parent, SpriteAtlas& atlas, 
										   EventBus2& bus, B2World& world)
{
	assert(parent.IsValid());
	auto rels = parent.GetRelations();
	assert(!rels.IsChild());

	Entity e = rels.AddChild();
	assert(e.IsValid());

	assert(atlas.IsLoaded());
	auto& rend = e.AddComponent<SpriteRenderableComponent>();

	TRY_ASSIGN(rend.sprite, atlas.LoadSprite(SDLite::Renderer(),
		{ .filepath = GetCrosshairSpritePath() }));

	e.AddComponent<CrosshairComponent>();

	assert(parent.HasComponent<Transform>());
	e.AddComponent<Transform>().position = parent.GetComponent<Transform>().position;

	auto evs = e.GetEvents(bus);

	EntityEvents::FilterDef filterDef = { .relevantEntity = parent.GetID() };

	TRY(evs.OnEvent([](const events::EntityPositionChanged& ev, Transform& tf,
					   CrosshairComponent& cr) {
		tf.position = ev.newPosition + cr.parentOffset;
	}, filterDef));

	TRY(evs.OnInput(GameControllerInputSource::RightStickAxis,
	[](const events::GameControllerInput& ev, Transform& tf,
		SpriteRenderableComponent& r, CrosshairComponent& cr) {
			const bool process = AxisOutsideDeadzone(ev.input.value.axis);
			if (!process)
			{
				r.profile.mods.alpha = 0;
				cr.active = false;
				cr.parentOffset = { 0.0f, 0.0f };
				return;
			}

			r.profile.mods.alpha = 255;
			cr.active = true;			
			cr.parentOffset = MapAxisToRadius(ev.input.value.axis, cr.radius);

	}, filterDef));

	//TRY(evs.OnInput(GameControllerInputSource::RightTrigger, 
	//	) { . })

	return e;
}

static Result<Entity> SetUpPlayerEntity(Entity& parent, SpriteAtlas& atlas,
										EventBus2& bus, B2World& world)
{
	auto e = ECS::CreateEntity();
	assert(e.IsValid());

	e.AddComponent<Transform>().position = SDLite::Window().GetLocalCenter<SDL_FPoint>();

	auto& rend = e.AddComponent<SpriteRenderableComponent>();
	TRY_ASSIGN(rend.sprite, atlas.LoadSprite(SDLite::Renderer(),
		{ .filepath = GetKnightSpritePath() }));



	TRY(SetUpCrosshairEntity(e, atlas, bus, world), crosshairEnt);
}



} // game