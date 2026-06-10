#include "Grapple.h"
#include "../../../ecs/EntityPhysics.h"
#include "../../../file/FilePathUtility.h"
#include <ranges>

namespace test {

static constexpr float kLinkCircleRadius = 5.0f;
static constexpr float kBallCircleRadius = 30.0f;

Result<LegIron> LegIron::Create(B2World& world, TextureRepository& repo, 
								Entity& connectingEnt, const Definition& def)
{
	assert(def.numLinks > 0);

	if (!connectingEnt.HasComponent<Transform>())
	{
		return MAKE_ERROR("Connecting Entity missing Transform component");
	}
	auto& connectingEntTf = connectingEnt.GetComponent<Transform>();
	const auto start = connectingEntTf.position;

	SDL_FPoint dir = { 
		def.headPos.x - start.x,
		def.headPos.y - start.y
	};
	float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);
	
	float adv = dist / static_cast<float>(def.numLinks);
	
	float invDist = 1.0f / dist;

	SDL_FPoint normed = { dir.x * invDist, dir.y * invDist };

	if (!connectingEnt.HasComponent<RigidBody>())
	{
		return MAKE_ERROR("Connecting Entity missing RigidBody component");
	}
	const auto& startBody = connectingEnt.GetComponent<RigidBody>().body.GetData();
	if (!startBody.IsValid())
	{
		return MAKE_ERROR("Connecting Entity body was invalid");
	}

	auto prevBody = startBody;

	LegIron legIron{};
	legIron.entities_.reserve(def.numLinks);
	legIron.joints_.reserve(def.numLinks);

	auto& spriteAtlas = repo.GetSpriteAtlas();
	Sprite circleSprite{};
	if (!spriteAtlas.HasSprite("circle_icon"))
	{
		TRY(ResourcePath::Sprite("shapes/circle_icon.png"), circlePath);
		TRY_ASSIGN(circleSprite, 
			spriteAtlas.LoadSprite(SDLite::Renderer(), { .filepath = std::move(circlePath) }));
	}
	else
	{
		circleSprite = spriteAtlas.GetSprite("circle_icon");
	}
	assert(circleSprite.resourceHandle.IsValid());

	for (size_t i = 0; i < def.numLinks; i++)
	{
		const bool isHead = i == def.numLinks - 1;
		const bool isPlayer = i == 0;

		SDL_FPoint linkPointPos{
			start.x + normed.x * adv * static_cast<float>(i + 1),
			start.y + normed.y * adv * static_cast<float>(i + 1)
		};

		auto& e = legIron.entities_.emplace_back() = ECS::CreateEntity();
		assert(e.IsValid());

		e.AddComponent(SpriteRenderableComponent{ 
			.sprite = circleSprite,
			.profile = {
				.drawOrder = 9999,
				.mods = { .color = RGB::FromSDLColor(SDLite::kColorBlack) },
				//.debugDraw = { .collider = { .on = true }}
			}
		});

		auto phys = e.GetPhysics(world);
		auto body = phys.AddBody(B2Body::Type::Dynamic, linkPointPos);
		assert(body.IsValid());
		assert(e.HasComponent<Transform>());

		e.GetComponent<Transform>().scale = (isHead ? SDL_FPoint{ 3.3f, 3.3f } : 
													  SDL_FPoint{ 0.6f, 0.6f });

		B2CollisionFilter filter{};
		filter.categories = def.legIronCategory;
		filter.categoryMask &= ~(def.playerCategory);
		filter.categoryMask &= ~(def.legIronCategory);

		auto sh = phys.AddColliderCircle((isHead ? kBallCircleRadius : kLinkCircleRadius), { 
			.settings = { 
				.density = (isHead ? 5.0f : 1.0f),
				.friction = 1.5f,
				.enableEvents = { .contact = true },
				.enableCollision = true
			},
			.filter = filter
		});	
		assert(sh.IsValid());

		if (isHead)
		{
			filter = { .categories = def.legIronCategory };
			filter.categoryMask &= ~(def.legIronCategory);

			auto sensorSh = phys.AddColliderCircle(kBallCircleRadius, {
				.settings { .enableEvents = { .sensor = true }, .isSensor = true},
				.filter = filter
			});
			assert(sensorSh.IsValid());
			assert(e.GetRelations().HasChildren());
		}

		const auto len = body.GetDistance(prevBody);

		auto joint = B2JointFactory::MakeDistanceJoint(prevBody.GetHandle(), body.GetHandle(), {
			.length {.rest = 0.8f, .max = len },
			.localAnchor {.a = (isPlayer ? SDL_FPoint{ 0.0f, 25.0f } : SDL_FPoint{ 0.0f, 0.0f }) },
			.spring {.enable = true, .hertz = 24.0f, .dampingRatio = 1.0f },
			.collideConnected = false
		});
		assert(joint.IsValid());

		legIron.joints_.emplace_back(joint);

		prevBody = body;
	}

	//assert(!legIron.entities_.empty());
	//auto headRels = legIron.entities_.back().GetRelations();

	//auto sensorChild = headRels.AddChild();
	//auto sensorPhys = sensorChild.GetPhysics(world);

	//sensorP

	return legIron;
}

std::vector<SDL_FPoint> LegIron::GetPoints() const
{
	return entities_ | std::views::transform([](const Entity& e) {
		assert(e.HasComponent<Transform>());
		return e.GetComponent<Transform>().position;
	}) | std::ranges::to<std::vector>();
}

Entity LegIron::GetBallEntity()
{
	return (!entities_.empty()) ? entities_.back() : Entity{};
}

const Entity LegIron::GetBallEntity() const
{
	return (!entities_.empty()) ? entities_.back() : Entity{};
}

//void GrappleDrawSystem::Update(float)
//{
//	if (!rope_)
//	{
//		return;
//	}
//
//	auto points = rope_->GetPoints();
//
//	const auto origColor = SDLite::Renderer().GetColor();
//	SDLite::Renderer().SetColor(SDLite::kColorWhite);
//	SDL_RenderDrawLinesF(SDLite::Renderer(), points.data(), points.size());
//	SDLite::Renderer().SetColor(origColor);
//}

} // test