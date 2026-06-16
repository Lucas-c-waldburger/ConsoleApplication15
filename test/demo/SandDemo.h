#pragma once
#include "../../ecs/Ecs.h"
#include "../Fixtures.h"
#include "../../components/builder/RigidBodyComponentBuilder.h"
#include "../../components/builder/ColliderComponentBuilder.h"
#include "../Premades.h"

namespace test {

static Entity MakeFloor(B2World& world, SDL_Color color = SDLite::kColorPurple, 
	float floorHeight = 200.0f, std::optional<float> botYOffset = {})
{
	auto e = ECS::CreateEntity();
	assert(e.IsValid());

	auto [winW, winH] = SDLite::Window().GetSize<float>();
	auto [winCenterX, winCenterY] = SDLite::Window().GetLocalCenter<SDL_FPoint>();

	e.AddComponent<Transform>();

	if (!botYOffset.has_value())
	{
		botYOffset = floorHeight;
	}

	auto& rigid = e.AddComponent(ComponentBuilder<RigidBody>{}
	.WithBodyParameters({
		.bodyType = B2Body::Type::Static,
		.position = {
			winCenterX,
			winH + (floorHeight / 2.0f) - *botYOffset
		}
	}).Build(world));

	e.AddComponent(ComponentBuilder<Collider>{}
	.WithColliderSettings({
		.enableEvents = true
	})
	.WithShapeParameters({
		.shapeType = B2Shape::Type::Polygon,
		.dimensions = Dimensions<float>{ winW, floorHeight }
	}).Build(rigid.body));

	e.AddComponent(SpriteRenderableComponent{
		.profile = {.debugDraw = {.collider = {
			.on = true,
			.color = color
		}}}
	});

	return e;
}

static std::vector<SDL_FPoint> CreateHourglassPixels(
	float totalHeightPx,
	float topRadiusPx,
	float waistRadiusPx,
	float bottomRadiusPx,
	int segmentsPerHalf)
{
	std::vector<SDL_FPoint> vertices;

	float halfHeight = totalHeightPx * 0.5f;

	// In SDL:
	// Top = -halfHeight
	// Bottom = +halfHeight

	// ---- Start at TOP-LEFT ----
	vertices.push_back({ -topRadiusPx, -halfHeight });

	// ---- Upper left curve (top Å® waist) ----
	for (int i = 1; i <= segmentsPerHalf; ++i)
	{
		float t = static_cast<float>(i) / segmentsPerHalf; // 0 Å® 1
		float y = -halfHeight + t * halfHeight;  // moving downward

		float blend = (1.0f - std::cos(float(M_PI) * t)) * 0.5f;
		float radius = topRadiusPx + (waistRadiusPx - topRadiusPx) * blend;

		vertices.push_back({ -radius, y });
	}

	// ---- Lower left curve (waist Å® bottom) ----
	for (int i = 1; i <= segmentsPerHalf; ++i)
	{
		float t = static_cast<float>(i) / segmentsPerHalf;
		float y = 0 + t * halfHeight;  // moving further downward

		float blend = (1.0f - std::cos(float(M_PI) * t)) * 0.5f;
		float radius = waistRadiusPx + (bottomRadiusPx - waistRadiusPx) * blend;

		vertices.push_back({ -radius, y });
	}

	// ---- Bottom edge ----
	vertices.push_back({ bottomRadiusPx, halfHeight });

	// ---- Lower right curve (bottom Å® waist) ----
	for (int i = segmentsPerHalf - 1; i >= 0; --i)
	{
		float t = static_cast<float>(i) / segmentsPerHalf;
		float y = 0 + t * halfHeight;

		float blend = (1.0f - std::cos(float(M_PI) * t)) * 0.5f;
		float radius = waistRadiusPx + (bottomRadiusPx - waistRadiusPx) * blend;

		vertices.push_back({ radius, y });
	}

	// ---- Upper right curve (waist Å® top) ----
	for (int i = segmentsPerHalf - 1; i >= 0; --i)
	{
		float t = static_cast<float>(i) / segmentsPerHalf;
		float y = -halfHeight + t * halfHeight;

		float blend = (1.0f - std::cos(float(M_PI) * t)) * 0.5f;
		float radius = topRadiusPx + (waistRadiusPx - topRadiusPx) * blend;

		vertices.push_back({ radius, y });
	}

	// Stop at TOP-RIGHT. Do NOT connect back to start.
	return vertices;
}

static Result<Entity> MakeHourglass(B2World& world, int windowBottomOffset, 
									SDL_Color color = SDLite::kColorWhite)
{
	auto e = ECS::CreateEntity();
	assert(e.IsValid());

	auto points = CreateHourglassPixels(
		600.0f,   // total height in pixels
		200.0f,   // top radius
		50.0f,    // waist radius
		200.0f,   // bottom radius
		32        // smoothness
	);

	auto [winW, winH] = SDLite::Window().GetSize<float>();
	auto [winCenterX, winCenterY] = SDLite::Window().GetLocalCenter<SDL_FPoint>();

	e.AddComponent<Transform>();

	auto& rigid = e.AddComponent(ComponentBuilder<RigidBody>{}
	.WithBodyParameters({
		.bodyType = B2Body::Type::Static,
		.position = {
			winCenterX,
			winH - windowBottomOffset - (600.0f / 2.0f)
		}
	}).Build(world));

	auto body = WriteAccessor<B2Body>{}(rigid.body);
	assert(body.IsValid());

	B2ChainDefinition chainDef{
		.points = points
	};
	TRY(body.AddChain(chainDef), chain);
	assert(chain.IsValid());

	auto segments = chain.GetSegments();

	auto rels = e.GetRelations();
	for (const auto& seg : segments)
	{
		auto child = rels.AddChild();
		assert(child.IsValid());

		child.AddComponent<Transform>();
		child.AddComponent<Collider>().shape = seg;
		child.AddComponent<SpriteRenderableComponent>()
			.profile.debugDraw.collider = {
				.on = true,
				.color = color
			};
	}

	return e;
}


static SDL_Color GetRainbowColor(float t)
{
	constexpr auto pi = static_cast<float>(M_PI);

	float r = std::sin(2.0f * pi * t + 0.0f) * 0.5f + 0.5f;
	float g = std::sin(2.0f * pi * t + 2.0f * pi / 3.0f) * 0.5f + 0.5f;
	float b = std::sin(2.0f * pi * t + 4.0f * pi / 3.0f) * 0.5f + 0.5f;

	return {
		static_cast<uint8_t>(r * 255),
		static_cast<uint8_t>(g * 255),
		static_cast<uint8_t>(b * 255),
		255
	};
}

static Entity SpawnSandParticle(B2World& world, SDL_FPoint spawnPos, 
								SDL_Color color)
{
	auto e = ECS::CreateEntity();

	e.AddComponent<Transform>();
	auto& debug = e.AddComponent<SpriteRenderableComponent>().profile.debugDraw;
	debug.collider.on = true;
	debug.collider.color = color;

	auto& body = e.AddComponent(ComponentBuilder<RigidBody>{}
	.WithBodyParameters({
			.bodyType = B2Body::Type::Dynamic,
			.position = spawnPos
	}).Build(world));

	e.AddComponent(ComponentBuilder<Collider>{}
	.WithColliderSettings({ 
		.enableEvents = false
	})
	.WithShapeParameters({
		.shapeType = B2Shape::Type::Polygon,
		.dimensions = Dimensions{ 17.0f, 17.0f },
	}).Build(body.body));

	return e;
}

class SandSpawner
{
public:
	explicit SandSpawner(B2World& world) : world_(&world) {}
	SandSpawner(B2World& world, float interval) : world_(&world), spawnInterval_(interval) {}

	void Update(float dt)
	{
		timeSinceLastSpawn_ += dt;
		deltaAccum_ += dt;

		if (timeSinceLastSpawn_ >= spawnInterval_)
		{
			if (sandCount_ >= maxSandCount_)
			{
				return;
			}

			assert(world_);
			assert(maxSandCount_ > 0);

			const SDL_Color color = GetRainbowColor(deltaAccum_);

			SpawnSandParticle(*world_, {
				.x = SDLite::Window().GetLocalCenter<SDL_FPoint>().x,
				.y = 0.0f
			}, color);

			++sandCount_;

			timeSinceLastSpawn_ = 0.0f;
		}
	}

private:
	B2World* world_ = nullptr;
	float deltaAccum_ = 0.0f;
	float timeSinceLastSpawn_ = 0.0f;
	float spawnInterval_ = 0.10f;
	size_t sandCount_ = 0;
	size_t maxSandCount_ = 300;
};

class MouseExploder
{
public:
	explicit MouseExploder(B2World& world, B2ExplosionDefinition def = {}) :
		world_(&world), explosionDef_(std::move(def))
	{
		auto e = ECS::CreateEntity();
		assert(e.IsValid());

		e.AddComponent<MouseState>();

		entityId_ = e.GetID();
	}

	void Update(float)
	{
		auto e = ECS::GetEntityByID(entityId_);
		if (!e.IsValid())
		{
			return;
		}
		if (!e.HasComponent<MouseState>())
		{
			return;
		}

		auto& mouse = e.GetComponent<MouseState>();

		const auto& leftInp = mouse.inputs[MouseInputSource::LeftButton];
		if (leftInp.state == InputState::Pressed)
		{
			assert(world_);

			explosionDef_.position = mouse.values.cursor.absolutePos;

			world_->Explode(explosionDef_);
		}
	}

private:
	B2World* world_ = nullptr;
	B2ExplosionDefinition explosionDef_;
	Entity_t entityId_ = kInvalidEntity;
};

static Result<Void> RunSandDemo(SceneFixture::SharedPtr& fixture)
{
	TRY(ResourcePath::Font("GoNotoKurrent-Regular.ttf"), fontPath);

	auto& fontAtlas = fixture->GetTextureRepository().GetFontAtlas();
	TRY(fontAtlas.LoadFont(fixture->GetRenderer(), {
		.filepath = std::move(fontPath),
		.fontSize = 24
		}), fontHandle);

	fixture->RegisterSystem<test::FPSReporter>(Phase::Setup,
		GlyphTextWriter{ .resourceHandle = fontHandle },
		SDLite::kColorWhite);
	fixture->RegisterSystem<test::SandSpawner>(Phase::Intent,
		fixture->GetWorld());
	fixture->RegisterSystem<test::MouseExploder>(Phase::Intent,
		fixture->GetWorld());

	auto floor = test::MakeFloor(fixture->GetWorld());
	TRY(test::MakeHourglass(fixture->GetWorld(), 200), hourglass);

	return fixture->RunGameLoop();
}


} // test