#include "Grapple.h"
#include "../../../ecs/EntityPhysics.h"
#include "../../../ecs/EntityEvents.h"
#include "../../../file/FilePathUtility.h"
#include "../platformer/Setup.h"
#include <ranges>

namespace test {

namespace {

} // unnamed

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

	Sprite spikeBallSprite{};
	if (!spriteAtlas.HasSprite("spike_ball"))
	{
		TRY(ResourcePath::Sprite("shapes/spike_ball.png"), spikeBallPath);
		TRY_ASSIGN(spikeBallSprite,
			spriteAtlas.LoadSprite(SDLite::Renderer(), { .filepath = std::move(spikeBallPath) }));
	}
	else
	{
		spikeBallSprite = spriteAtlas.GetSprite("spike_ball");
	}
	assert(spikeBallSprite.resourceHandle.IsValid());

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
			.sprite = (isHead ? spikeBallSprite : circleSprite),
			.profile = {
				.drawOrder = 9999,
				.mods = {.color = (isHead ? RGB{} : RGB::FromSDLColor(SDLite::kColorBlack)) },
				//.debugDraw = { .collider = { .on = true }}
			}
		});

		auto phys = e.GetPhysics(world);
		auto body = phys.AddBody(B2Body::Type::Dynamic, linkPointPos);
		assert(body.IsValid());
		assert(e.HasComponent<Transform>());

		e.GetComponent<Transform>().scale = (isHead ? SDL_FPoint{ 0.2f, 0.2f } : 
													  SDL_FPoint{ 0.6f, 0.6f });

		B2CollisionFilter filter{};
		filter.categories = def.legIronCategory;
		filter.categoryMask &= ~(def.playerCategory);
		filter.categoryMask &= ~(def.legIronCategory);

		auto sh = phys.AddColliderCircle((isHead ? kBallCircleRadius : kLinkCircleRadius), { 
			.settings = { 
				.density = (isHead ? 10.0f : 1.0f),
				.friction = (isHead ? 200.0f : 1.5f),
				.enableEvents = { .contact = true },
				.enableCollision = true
			},
			.filter = filter
		});	
		assert(sh.IsValid());

		if (isHead)
		{
			filter = { .categories = def.ballSensorCategory };
			filter.categoryMask &= ~(def.legIronCategory);

			auto sensorSh = phys.AddColliderCircle(kBallCircleRadius + 5.0f, {
				.settings { .enableEvents = { true, true, true }, .isSensor = true},
				.filter = filter
			});
			assert(sensorSh.IsValid());
			assert(e.GetRelations().HasChildren());
		}

		const auto len = body.GetDistance(prevBody);
		legIron.defaultJointLength_ = 0.8f;

		auto joint = B2JointFactory::MakeDistanceJoint(prevBody.GetHandle(), body.GetHandle(), {
			.length {.rest = 0.8f, .max = len },
			.localAnchor {.a = (isPlayer ? SDL_FPoint{ 0.0f, 25.0f } : SDL_FPoint{ 0.0f, 0.0f }) },
			.spring {.enable = true, .hertz = 24.0f, .dampingRatio = 1.0f },
			.collideConnected = false
		});
		assert(joint.IsValid());

		legIron.joints_.emplace_back(joint);

		prevBody = body;

		if (isHead)
		{
			const auto pos = body.GetPosition();
			body.SetPosition({ pos.x + 300.0f, pos.y });
			body.SetLinearVelocity({ 0.0f, 0.0f });
			body.SetAngularVelocity(0.0f);
			body.SetGravityScale(2.5f);
		}
	}

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

Entity LegIron::GetBallSensor()
{
	if (auto e = GetBallEntity(); e.IsValid())
	{
		if (auto ch = e.GetRelations().GetChildren(); !ch.empty())
		{
			assert(ch.size() == 1);
			return ch.front();
		}
	}

	return {};
}

void LegIron::SetJointRestLength(float len)
{
	len = std::clamp(len, 0.0001f, defaultJointLength_);

	ForEachJoint([len](B2DistanceJoint& j) {
		j.SetRestLength(len);
	});
}

float LegIron::GetJointRestLength() const noexcept
{
	return (!joints_.empty()) ? joints_.back().GetRestLength() : 0.0f;
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