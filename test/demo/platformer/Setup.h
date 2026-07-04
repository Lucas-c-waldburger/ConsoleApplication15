#pragma once
#include <optional>
#include "Common.h"
#include "GirlStateCoordinator.h"
#include "../../../ecs/Ecs.h"
#include "../../../ecs/EntityPhysics.h"
#include "../../../file/FilePathUtility.h"
#include "../../Fixtures.h"
#include "../../../ecs/EntityEvents.h"
#include "../../../components/builder/RigidBodyComponentBuilder.h"
#include "../../../components/builder/ColliderComponentBuilder.h"
#include "../../../events/data/EventDataIncludes.h"
#include "../../../sdl/SDLUtils.h"
#include "../../../core/SizedEnum.h"
#include "../SandDemo.h"
#include "../grapple/Grapple.h"

namespace test {

static Result<SpriteDescriptorPackage> GetGirlSpriteDescriptorPackage()
{
	TRY(ResourcePaths::SpriteDirectory("girl/idle"), idlePaths);
	TRY(ResourcePaths::SpriteDirectory("girl/idleS"), idleSPaths);
	TRY(ResourcePaths::SpriteDirectory("girl/walk"), walkPaths);
	TRY(ResourcePaths::SpriteDirectory("girl/walkS"), walkSPaths);
	TRY(ResourcePaths::SpriteDirectory("girl/jump"), jumpPaths);
	TRY(ResourcePaths::SpriteDirectory("girl/jumpS"), jumpSPaths);
	TRY(ResourcePaths::SpriteDirectory("girl/land"), landPaths);
	TRY(ResourcePaths::SpriteDirectory("girl/attack_A"), attackAPaths);
	TRY(ResourcePaths::SpriteDirectory("girl/attack_B"), attackBPaths);
	TRY(ResourcePaths::SpriteDirectory("girl/fall"), fallPaths);
	TRY(ResourcePaths::SpriteDirectory("girl/fallS"), fallSPaths);
	TRY(ResourcePaths::SpriteDirectory("girl/roll"), rollPaths);
	TRY(ResourcePaths::SpriteDirectory("girl/dash"), dashPaths);
	TRY(ResourcePaths::SpriteDirectory("girl/sheath"), sheathPaths);
	TRY(ResourcePaths::SpriteDirectory("girl/push"), pushPaths);

	static constexpr auto toDescriptors = []
	(std::string_view seriesName, std::vector<std::string>&& paths) {
		return SpriteDescriptors{
			.data = paths | std::views::transform([](auto&& path) {
				return SpriteDescriptor{.filepath = std::move(path) };
			}) | std::ranges::to<std::vector>(),
			.seriesName = std::string{seriesName}
		};
	};

	return SpriteDescriptorPackage{
		toDescriptors("girl_idle", std::move(idlePaths)),
		toDescriptors("girl_idle_s", std::move(idleSPaths)),
		toDescriptors("girl_walk", std::move(walkPaths)),
		toDescriptors("girl_walk_s", std::move(walkSPaths)),
		toDescriptors("girl_jump", std::move(jumpPaths)),
		toDescriptors("girl_jump_s", std::move(jumpSPaths)),
		toDescriptors("girl_land", std::move(landPaths)),
		toDescriptors("girl_attack_A", std::move(attackAPaths)),
		toDescriptors("girl_attack_B", std::move(attackBPaths)),
		toDescriptors("girl_fall", std::move(fallPaths)),
		toDescriptors("girl_fall_s", std::move(fallSPaths)),
		toDescriptors("girl_roll", std::move(rollPaths)),
		toDescriptors("girl_dash", std::move(dashPaths)),
		toDescriptors("girl_sheath", std::move(sheathPaths)),
		toDescriptors("girl_push", std::move(pushPaths))
	};
}

static Result<Void> LoadFonts(FontAtlas& atlas, SDL_Renderer* renderer)
{
	TRY(ResourcePaths::FontDirectory(""), fontPaths);

	static constexpr auto toFontDescriptors = [](auto&& paths) {
		return paths | std::views::transform([](auto&& path) {
			return FontDescriptor{
				.filepath = std::move(path),
				.fontSize = 28
			};
		}) | std::ranges::to<std::vector>();
	};

	return atlas.LoadFonts(renderer, toFontDescriptors(std::move(fontPaths)));
}

static Result<Void> LoadGirlSprites(SpriteAtlas& atlas, SDL_Renderer* renderer)
{
	TRY(GetGirlSpriteDescriptorPackage(), spriteDescriptors);

	for (auto&& descriptor : spriteDescriptors)
	{
		TRY(atlas.LoadSprites(renderer, std::move(descriptor)));
	}

	return kVoid;
}

class GirlStateReporter
{
public:
	static constexpr std::string_view kIdleStateStr = "Idle";
	static constexpr std::string_view kWalkStateStr = "Walking";
	static constexpr std::string_view kJumpStateStr = "Jumping";
	static constexpr std::string_view kLandStateStr = "Landing";
	static constexpr std::string_view kFallStateStr = "Falling";
	static constexpr std::string_view kAttackStateStr = "Attacking";

	static constexpr std::string_view kTextDrawFmt =
		"Current State: {}\n---------------\n"
		"Velocity: x:{:.2f}, y:{:.2f}\n---------------\n"
		"Jump Flag: {}\n---------------\n"
		"Ground: {}\nWall: {}\nCeiling: {}\nEnemy: {}";

	explicit GirlStateReporter(Entity& target,
		GlyphTextWriter&& textWriter,
		SDL_Color textColor = SDLite::kColorBlack,
		Dimensions<int> dimensions = { 200, 300 },
		std::optional<SDL_FPoint> location = {})
	{
		assert(target.IsValid());
		assert(target.HasComponent<GirlState>());

		lastState_ = target.GetComponent<GirlState>();

		auto e = ECS::CreateEntity();
		assert(e.IsValid());

		if (!location.has_value())
		{
			location = { 40.0f, 40.0f };
		}

		e.AddComponent<Transform>().position = *location;

		e.AddComponent(TextRenderableComponent{
			.writer = std::move(textWriter),
			.formatting = {
				.bounds = dimensions,
				.align = TextAlign::Left,
				.scaleToBounds = false
			},
			.profile = {
				.drawOrder = 1000,
				.mods = {
					.color = { textColor.r, textColor.g, textColor.b }
				}
			}
			});

		selfId_ = e.GetID();
		targetId_ = target.GetID();
	}

	void Update(float)
	{
		auto self = ECS::GetEntityByID(selfId_);
		if (!(self.IsValid() && self.HasComponent<TextRenderableComponent>()))
		{
			return;
		}
		auto target = ECS::GetEntityByID(targetId_);
		if (!(target.IsValid() && target.HasComponents<GirlState, RigidBody>()))
		{
			return;
		}

		auto [state, rigid] = target.GetComponents<GirlState, RigidBody>();

		const auto& cats = state.collidingCategories;

		auto& writer = self.GetComponent<TextRenderableComponent>().writer;

		using Cat = ObjectCategory;
		auto vel = rigid.body.GetData().GetLinearVelocity();
		if (std::abs(lastVelocity_.x - vel.x) < 0.01 && std::abs(lastVelocity_.y - vel.y) < 0.01)
		{
			vel = lastVelocity_;
		}

		writer.text = std::format(kTextDrawFmt,
			GetAnimStateString(state.animation),
			vel.x, vel.y,
			(state.action.jumpIntent == InputState::Pressed ? "true" : "false"),
			cats[Cat::Ground], cats[Cat::Wall],
			cats[Cat::Ceiling], cats[Cat::Enemy]);

		lastState_ = state;
		lastVelocity_ = vel;
	}

	static constexpr std::string_view GetAnimStateString(GirlState::Animation anim)
	{
		using enum GirlState::Animation;
		static constexpr std::string_view kUnknown = "<Unknown>";
		switch (anim)
		{
		case Idle:		return kIdleStateStr;
		case Walking:	return kWalkStateStr;
		case Jumping:	return kJumpStateStr;
		case Landing:	return kLandStateStr;
		case Attacking: return kAttackStateStr;
		case Falling:	return kFallStateStr;
		default:		return kUnknown;
		}
	}

private:
	Entity_t selfId_ = kInvalidEntity;
	Entity_t targetId_ = kInvalidEntity;
	GirlState lastState_;
	SDL_FPoint lastVelocity_ = { 0.0f, 0.0f };
};

static Result<Void>
SetUpGirlStateReporter(Entity& target, SceneFixture::SharedPtr& fixture)
{
	auto& atlas = fixture->GetTextureRepository().GetFontAtlas();

	TRY(LoadFonts(atlas, fixture->GetRenderer()));

	auto writer = atlas.GetTextWriter("GoNotoKurrent-Bold");
	if (!writer.resourceHandle.IsValid())
	{
		return MAKE_ERROR("Font not found");
	}

	fixture->RegisterSystem<GirlStateReporter>(
		Phase::Input, target, std::move(writer), SDLite::kColorWhite
	);

	return kVoid;
}

static Entity MakeStaticBox(B2World& world, SDL_Color color,
							Dimensions<float> dims,
							SDL_FPoint posOffset = { 0.0f, 0.0f })
{
	auto e = ECS::CreateEntity();
	assert(e.IsValid());

	auto [winW, winH] = SDLite::Window().GetSize<float>();
	auto winCenter = SDLite::Window().GetLocalCenter<SDL_FPoint>();

	e.AddComponent<Transform>();

	auto& rigid = e.AddComponent(ComponentBuilder<RigidBody>{}
	.WithBodyParameters({
		.bodyType = B2Body::Type::Static,
		.position = winCenter + posOffset,
		}).Build(world));

	e.AddComponent(ComponentBuilder<Collider>{}
	.WithColliderSettings({
		.enableEvents = true
		})
		.WithShapeParameters({
			.shapeType = B2Shape::Type::Polygon,
			.dimensions = dims,
			}).Build(rigid.body));

	e.AddComponent(SpriteRenderableComponent{
		.profile = {.debugDraw = {.collider = {
			.on = true,
			.color = color
		}}}
		});

	return e;
}

static Result<Void> SetUpEnvironment(SceneFixture::SharedPtr& fixture)
{
	TRY(ResourcePath::Sprite("environment/caverns/background.png"), backgroundPath);
	TRY(ResourcePath::Sprite("environment/caverns/back-walls.png"), backWallsPath);
	TRY(ResourcePath::Sprite("environment/caverns/tiles.png"), tilesPath);

	auto& atlas = fixture->GetTextureRepository().GetSpriteAtlas();
	auto* renderer = fixture->GetRenderer();

	TRY(atlas.LoadSprite(renderer, { .filepath = std::move(backgroundPath) }),
		backgroundSprite);
	TRY(atlas.LoadSprite(renderer, { .filepath = std::move(backWallsPath) }),
		backWallSprite);
	TRY(atlas.LoadSprite(renderer, { .filepath = std::move(tilesPath) }),
		tilesSprite);

	auto winCenter = SDLite::Window().GetLocalCenter<SDL_FPoint>();
	auto [winW, winH] = SDLite::Window().GetSize();

	// background
	auto backgroundSpriteW = backgroundSprite.plot.rect.w;
	auto backgroundSpriteH = backgroundSprite.plot.rect.h;

	float uniformScale = static_cast<float>(winH) /
		static_cast<float>(backgroundSpriteH);
	float newW = backgroundSpriteW * uniformScale;
	float halfW = newW / 2.0f;
	float x = halfW;

	while (x + halfW < static_cast<float>(winW))
	{
		auto backgroundEnt = ECS::CreateEntity();

		backgroundEnt.AddComponent(Transform{
			.position = { x, winCenter.y },
			.scale = { uniformScale, uniformScale }
			});

		backgroundEnt.AddComponent(SpriteRenderableComponent{
			.sprite = backgroundSprite,
			.profile = {.drawOrder = 50 }
			});

		x += newW;
	}

	// backwalls
	auto backWallEnt = ECS::CreateEntity();

	auto backWallSpriteW = backWallSprite.plot.rect.w;
	auto backWallSpriteH = backWallSprite.plot.rect.h;

	SDL_FPoint backWallScale = {
		static_cast<float>(winW) / static_cast<float>(backWallSpriteW),
		static_cast<float>(winH) / static_cast<float>(backWallSpriteH)
	};

	backWallEnt.AddComponent(Transform{
		.position = winCenter,
		.scale = backWallScale
		});

	backWallEnt.AddComponent(SpriteRenderableComponent{
		.sprite = backWallSprite,
		.profile = {.drawOrder = 100 }
		});

	// terrain tiles
	auto terrainTilesEnt = ECS::CreateEntity();

	auto terrainTilesSpriteW = tilesSprite.plot.rect.w;
	auto terrainTilesSpriteH = tilesSprite.plot.rect.h;

	SDL_FPoint terrainTilesScale = {
		static_cast<float>(winW) / static_cast<float>(terrainTilesSpriteW),
		static_cast<float>(winH) / static_cast<float>(terrainTilesSpriteH)
	};

	terrainTilesEnt.AddComponent(Transform{
		.position = winCenter,
		.scale = terrainTilesScale
		});

	terrainTilesEnt.AddComponent(SpriteRenderableComponent{
		.sprite = tilesSprite,
		.profile = {.drawOrder = 150 }
		});

	// floor
	auto floorEnt = MakeFloor(fixture->GetWorld(),
		SDLite::kColorGreen, 50.0f, 169.0f);
	assert(floorEnt.IsValid());

	floorEnt.AddComponent(ObjectCategory{ .value = ObjectCategory::Ground });

	const Dimensions<float> wallDims = {
		40.0f,
		96.0f * terrainTilesScale.y
	};
	const SDL_FPoint wall1PosOffset = {
		-139.0f * terrainTilesScale.x,
		172.0f
	};
	const SDL_FPoint wall2PosOffset = {
		122.2f * terrainTilesScale.x,
		172.0f
	};

	auto wallEnt1 = MakeStaticBox(fixture->GetWorld(), SDLite::kColorOrange,
		wallDims, wall1PosOffset);
	assert(wallEnt1.IsValid());
	wallEnt1.AddComponent(ObjectCategory{ .value = ObjectCategory::Wall });

	auto wallEnt2 = MakeStaticBox(fixture->GetWorld(), SDLite::kColorOrange,
		wallDims, wall2PosOffset);
	assert(wallEnt2.IsValid());
	wallEnt2.AddComponent(ObjectCategory{ .value = ObjectCategory::Wall });

	const Dimensions<float> platformDims = {
		996.5f,
		40.0f
	};
	const SDL_FPoint platform1PosOffset = {
		-30.8f, 20.0f
	};

	auto platformEnt1 = MakeStaticBox(fixture->GetWorld(), SDLite::kColorGreen,
		platformDims, platform1PosOffset);
	assert(platformEnt1.IsValid());
	platformEnt1.AddComponent(ObjectCategory{ .value = ObjectCategory::Ground });

	return kVoid;
}

struct ResolvedCollisionData
{
	CollisionData self;
	CollisionData other;
};

template <typename T> requires type_in_list_v<T, events::CollisionEventGroup>
static std::optional<ResolvedCollisionData>
ResolveCollisionData(const Collider& selfCollider, const T& ev)
{
	const auto& selfHandle = selfCollider.shape.GetData().GetHandle();
	if (ev.a.shapeHandle == selfHandle)
	{
		return ResolvedCollisionData{ .self = ev.a, .other = ev.b };
	}
	if (ev.b.shapeHandle == selfHandle)
	{
		return ResolvedCollisionData{ .self = ev.b, .other = ev.a };
	}

	return std::nullopt;
}

static Entity FindOwningBodyEntity(const CollisionData& collisionData)
{
	auto shapeEnt = ECS::GetEntityByID(collisionData.entity);
	if (!shapeEnt.IsValid())
	{
		return {};
	}

	assert(shapeEnt.HasComponent<Collider>());

	if (shapeEnt.HasComponent<RigidBody>())
	{
		assert(shapeEnt.GetComponent<RigidBody>().body.GetData().OwnsShape(
			   collisionData.shapeHandle));

		return shapeEnt;
	}

	auto& colliderShape = shapeEnt.GetComponent<Collider>().shape;
	auto parentBodyHandle = colliderShape.GetData().GetParentBodyHandle();

	auto rels = shapeEnt.GetRelations();
	assert(rels.IsChild());

	auto parent = rels.GetParent();

	assert(parent.IsValid());
	assert(parent.HasComponent<RigidBody>());
	assert(parent.GetComponent<RigidBody>().body.GetData().GetHandle() ==
		   parentBodyHandle);

	return parent;
}

static ObjectCategory::Type GetEntityObjectCategory(Entity_t entityId)
{
	auto e = ECS::GetEntityByID(entityId);
	if (!e.IsValid())
	{
		return ObjectCategory::Unknown;
	}

	return e.HasComponent<ObjectCategory>()
		? e.GetComponent<ObjectCategory>().value
		: ObjectCategory::Unknown;
}

static SDL_FPoint ComputeSwordHitImpulse(SDL_FPoint normal, const RigidBody& otherRigid)
{
	const auto& otherBody = otherRigid.body.GetData();

	return normal * (otherBody.GetMass() * kSwordHitImpulse);
}

static constexpr bool IsSwordActive(const SpriteRenderableComponent& rend)
{
	return rend.profile.debugDraw.collider.on == true;
}

static void SetUpSwordEntityCallbacks(Entity& sword, EventBus& bus)
{
	auto evs = sword.GetEvents(bus);

	evs.OnEvent([](const events::SensorCollisionBegin& ev, Collider& collider,
				   CollisionCache& collisionCache) {
		auto resolved = ResolveCollisionData(collider, ev);
		if (!resolved.has_value())
		{
			return;
		}

		collisionCache.data.emplace(resolved->other.shapeHandle, resolved->other.entity);
	});

	evs.OnEvent([](const events::SensorCollisionEnd& ev, Collider& collider,
				   CollisionCache& collisionCache) {
		auto resolved = ResolveCollisionData(collider, ev);
		if (!resolved.has_value())
		{
			return;
		}

		collisionCache.data.erase(resolved->other.shapeHandle);
	});
}

static void SetUpGirlEntityCallbacks(Entity& e, EventBus& bus)
{
	auto evs = e.GetEvents(bus);

	auto addCollideLambda = [&]<typename Ev, auto memPtr>() {
		evs.OnEvent([](const Ev& ev, Collider& col, GirlState& st) {
			if (auto resolved = ResolveCollisionData(col, ev)) {
				(st.collidingCategories.*memPtr)(GetEntityObjectCategory(resolved->other.entity));
			}
		});
	};

	static constexpr auto inc = &CollisionCategoryTracker::Increment;
	static constexpr auto dec = &CollisionCategoryTracker::Decrement;

	addCollideLambda.template operator()<events::ContactCollisionBegin, inc>();
	addCollideLambda.template operator()<events::ContactCollisionEnd, dec>();
	addCollideLambda.template operator()<events::SensorCollisionBegin, inc>();
	addCollideLambda.template operator()<events::SensorCollisionEnd, dec>();

	evs.OnEvent([](const events::GameControllerConnected& ev,
		GameControllerState& gcState)
		{
			if (gcState.joystickID == GameController::kInvalidJoystickID)
			{
				gcState.joystickID = ev.joystickID;
			}
		});
	evs.OnEvent([](const events::GameControllerDisconnected& ev,
		GameControllerState& gcState)
		{
			if (gcState.joystickID == ev.joystickID)
			{
				gcState.joystickID = GameController::kInvalidJoystickID;
			}
		});

	evs.OnInput(GameControllerInputSource::LeftStickAxis,
		[](const events::GameControllerInput& ev, GirlState& state)
		{
			if (state.action.moveIntent.has_value())
			{
				return;
			}

			const int axisValX = ev.input.value.axis.x;
			const int axisValY = ev.input.value.axis.y;
			if (std::abs(axisValX) < kGirlControllerAxisDeadzone &&
				std::abs(axisValY) < kGirlControllerAxisDeadzone)
			{
				return;
			}

			state.action.moveIntent = SDL_FPoint{
				static_cast<float>(axisValX),
				static_cast<float>(axisValY)
			};
		});

	using Act = GirlActionIntent;
	using Src = GameControllerInputSource;

	auto addBasicIntentLambda = [&evs]<auto intentPtr, Src src>() { 
		evs.OnInput(src, [](const events::GameControllerInput& ev, GirlState& state) {
			state.action.*intentPtr = ev.input.state;
		});
	};

	addBasicIntentLambda.template operator()<&Act::jumpIntent, Src::A>();
	addBasicIntentLambda.template operator()<&Act::carryIntent, Src::B>();
	addBasicIntentLambda.template operator()<&Act::attackIntent, Src::X>();
	addBasicIntentLambda.template operator()<&Act::retractIntent, Src::RightTrigger>();
	addBasicIntentLambda.template operator()<&Act::ballFreezeIntent, Src::LeftTrigger>();
}

static Result<Entity> MakeCrateEntity(SceneFixture::SharedPtr& fixture,
									  SDL_FPoint startingPos)
{
	static constexpr float kCrateScale = 2.0f;

	TRY(ResourcePath::Sprite("environment/sCrate.png"), cratePath);
	TRY(fixture->GetTextureRepository().GetSpriteAtlas().LoadSprite(
		fixture->GetRenderer(), SpriteDescriptor{ .filepath = std::move(cratePath) }
	), crateSprite);

	auto e = ECS::CreateEntity();
	assert(e.IsValid());

	auto phys = e.GetPhysics(fixture->GetWorld());
	auto body = phys.AddBody({
		.bodyType = B2Body::Type::Dynamic,
		.position = startingPos,
		.gravityScale = 3.0f 
	});
	assert(body.IsValid());

	B2CollisionFilter filter{ .categories = ObjectCategory::Enemy };

	auto shape = phys.AddColliderBox(
		{ 48.0f * kCrateScale, 48.0f * kCrateScale },
		{ .settings = {.enableEvents = {true, true, true} }, .filter = filter });
	assert(shape.IsValid());

	e.GetComponent<Transform>().scale = { kCrateScale, kCrateScale };

	e.AddComponent(SpriteRenderableComponent{
		.sprite = crateSprite,
		.profile = {
			.drawOrder = 500,  
			.debugDraw = { .collider = {.on = true} },
		},
	});

	e.AddComponent<ObjectCategory>().value = ObjectCategory::Enemy;

	return e;
}


static Result<Entity> MakeGirlEntity(SceneFixture::SharedPtr& fixture,
									 SDL_FPoint startingPos)
{
	TRY(LoadGirlSprites(fixture->GetTextureRepository().GetSpriteAtlas(),
		fixture->GetRenderer()));

	auto e = ECS::CreateEntity();
	assert(e.IsValid());

	e.AddComponent(Transform{ .scale = kGirlSpriteScale });

	e.AddComponent(SpriteRenderableComponent{
		.profile = {.drawOrder = 500 }
	});
	e.AddComponent<SpriteAnimationComponent>().spriteSeriesName = "girl_idle";

	auto& rigid = e.AddComponent(ComponentBuilder<RigidBody>{}
	.WithBodyParameters({
		.bodyType = B2Body::Type::Dynamic,
		.position = startingPos,
		.gravityScale = kGirlNormalGravityScale,
		.fixedRotation = true
	}).Build(fixture->GetWorld()));

	B2CollisionFilter filter{};
	filter.categories = ObjectCategory::Player;
	filter.categoryMask &= ~(ObjectCategory::PlayerSword);
	//filter.categoryMask |= ObjectCategory::PlayerLegIron;

	e.AddComponent(ComponentBuilder<Collider>{}
	.WithShapeParameters({
		.shapeType = B2Shape::Type::Polygon,
		.dimensions = Dimensions<float>{
			kGirlIdleHitboxDimensions.w * kGirlSpriteScale.x,
			kGirlIdleHitboxDimensions.h * kGirlSpriteScale.y
		}
	}).WithColliderSettings({
		.friction = kGirlColliderFriction,
		.enableEvents = {true, true, true}
	})
	.WithFilter(filter).Build(rigid.body));

	e.AddComponent<GameControllerState>();
	e.AddComponent<MoveTargets>() = kGirlBaseMoveTargets;
	e.AddComponent<AnimationDeltas>() = kGirlBaseAnimationDeltas;
	e.AddComponent(GirlState{ .animation = GirlState::Animation::Idle });

	SetUpGirlEntityCallbacks(e, fixture->GetEventBus());

	return e;
}

static Result<Entity> MakeSwordEntity(SceneFixture::SharedPtr& fixture, Entity& girl)
{
	assert(girl.HasComponent<RigidBody>());
	assert(girl.HasComponent<Collider>());

	auto [gTf, gRigid, gCollider] = girl.GetComponents<Transform, RigidBody, Collider>();
	const SDL_FPoint gScale = gTf.scale;
	const auto& gBody = WriteAccessor<B2Body>{}(gRigid.body);
	const auto& gShape = WriteAccessor<B2Shape>{}(gCollider.shape);

	auto sw = ECS::CreateEntity();
	assert(sw.IsValid());

	sw.AddComponent(Transform{ .scale = gScale });

	auto& swBody = sw.AddComponent(ComponentBuilder<RigidBody>{}
	.WithBodyParameters({
		.bodyType = B2Body::Type::Static,
		.position = gBody.GetPosition()
	}).Build(fixture->GetWorld()));

	sw.AddComponent(ComponentBuilder<Collider>{}
	.WithFilter({
		.categories = ObjectCategory::PlayerSword,
		.categoryMask = ObjectCategory::Enemy
	})
	.WithShapeParameters({
		.shapeType = B2Shape::Type::Polygon,
		.dimensions = Dimensions<float>{
			kGirlSwordHitboxDimensions.w * gScale.x,
			kGirlSwordHitboxDimensions.h * gScale.y
		}
	})
	.WithColliderSettings({
		.enableEvents = { true, true, true },
		.isSensor = true
	}).Build(swBody.body));

	sw.AddComponent<SpriteRenderableComponent>();
	sw.AddComponent<CollisionCache>();

	SetUpSwordEntityCallbacks(sw, fixture->GetEventBus());

	return sw;
}

template <typename Ev, auto memPtr>
inline void TrackerLambda(const Ev& ev, Collider& col, CollisionCategoryTracker& tr)
{
	if (auto resolved = ResolveCollisionData(col, ev))
	{
		(tr.*memPtr)(GetEntityObjectCategory(resolved->other.entity));
	}
}

inline Result<Void> RegisterCollisionCategoryTrackerCallbacks(EntityEvents& entEvs)
{
	static constexpr auto makeLambda = []<typename Ev, auto memPtr>() {
		return [](const Ev& ev, Collider& col, CollisionCategoryTracker& tr) {
			if (auto resolved = ResolveCollisionData(col, ev))
			{
				(tr.*memPtr)(GetEntityObjectCategory(resolved->other.entity));
			}
		};
	};

	static constexpr auto inc = &CollisionCategoryTracker::Increment;
	static constexpr auto dec = &CollisionCategoryTracker::Decrement;

	TRY(entEvs.OnEvent(&TrackerLambda<events::ContactCollisionBegin, inc>));
	TRY(entEvs.OnEvent(&TrackerLambda<events::ContactCollisionEnd, dec>));
	TRY(entEvs.OnEvent(&TrackerLambda<events::SensorCollisionBegin, inc>));
	TRY(entEvs.OnEvent(&TrackerLambda<events::SensorCollisionEnd, dec>));

	//TRY(entEvs.OnEvent(makeLambda.template operator()<events::ContactCollisionBegin, inc>()));
	//TRY(entEvs.OnEvent(makeLambda.template operator()<events::ContactCollisionEnd, dec>()));
	//TRY(entEvs.OnEvent(makeLambda.template operator()<events::SensorCollisionBegin, inc>()));
	//TRY(entEvs.OnEvent(makeLambda.template operator()<events::SensorCollisionEnd, dec>()));

	return kVoid;
}

template <SomeEventData BegEv, SomeEventData EndEv>
inline Result<Void> RegisterCollisionCacheCallbacks(EntityEvents& entEvs)
{
	TRY(entEvs.OnEvent([](const BegEv& ev, Collider& col, CollisionCache& cache) {
		if (auto resolved = ResolveCollisionData(col, ev))
		{
			cache.data.try_emplace(resolved->other.shapeHandle, resolved->other.entity);
		}
	}));
	TRY(entEvs.OnEvent([](const EndEv& ev, Collider& col, CollisionCache& cache) {
		if (auto resolved = ResolveCollisionData(col, ev))
		{
			cache.data.erase(resolved->other.shapeHandle);
		}
	}));

	return kVoid;
}

inline Result<LegIron> MakeLegIronEntity(SceneFixture::SharedPtr& fixture, Entity& girl, SDL_FPoint girlStartPos)
{
	TRY(LegIron::Create(fixture->GetWorld(), fixture->GetTextureRepository(),
		girl, {
			.headPos = { girlStartPos.x + 5.0f, girlStartPos.y},
			.numLinks = 11,
			.playerCategory = ObjectCategory::Player,
			.legIronCategory = ObjectCategory::PlayerSword,
			.ballSensorCategory = ObjectCategory::PlayerLegIron
		}), legIron);

	auto ballE = legIron.GetBallEntity();
	assert(ballE.IsValid());
	assert(ballE.HasComponent<Collider>());

	//ballSensor.AddComponent<CollisionCache>();
	legIron.GetBallSensor().AddComponent<ObjectCategory>().value = ObjectCategory::PlayerLegIron;
	ballE.AddComponent<ObjectCategory>().value = ObjectCategory::PlayerLegIron;
	ballE.AddComponent<CollisionCategoryTracker>();

	auto evs = ballE.GetEvents(fixture->GetEventBus());

	//evs.OnEvent([](const events::SensorCollisionBegin& ev, Collider& col, CollisionCache& cache) {
	//	if (auto resolved = ResolveCollisionData(col, ev))
	//	{
	//		cache.data.try_emplace(resolved->other.shapeHandle, resolved->other.entity);
	//	}
	//	});
	//evs.OnEvent([](const events::SensorCollisionEnd& ev, Collider& col, CollisionCache& cache) {
	//	if (auto resolved = ResolveCollisionData(col, ev))
	//	{
	//		cache.data.erase(resolved->other.shapeHandle);
	//	}
	//	});
	TRY(RegisterCollisionCategoryTrackerCallbacks(evs));
	//TRY((RegisterCollisionCacheCallbacks<events::SensorCollisionBegin, 
	//									 events::SensorCollisionEnd>(evs)));

	return legIron;
}

} // test