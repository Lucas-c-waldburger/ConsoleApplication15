#pragma once
#include <cmath>
#include "../../physics/B2World.h"
#include "../../ecs/Ecs.h"
#include "../../components/builder/RigidBodyComponentBuilder.h"

namespace game {

constexpr inline float RandomGradient(int x)
{
	x = (x << 13) ^ x;
	return 1.0f - ((x * (x * x * 15731 + 789221) + 1376312589)
		& 0x7fffffff) / 1073741824.0f;
}

float SmoothStep(float t) 
{
	return t * t * (3.0f - 2.0f * t);
}

float Perlin1D(float x) 
{
    int x0 = static_cast<int>(std::floor(x));
    int x1 = x0 + 1;

    float t = x - static_cast<float>(x0);
    float fade = SmoothStep(t);

    float g0 = RandomGradient(x0);
    float g1 = RandomGradient(x1);

    float v0 = g0 * (t);
    float v1 = g1 * (t - 1.0f);

    return std::lerp(v0, v1, fade);
}

static constexpr float kTerrainHeightFreq = 0.02f;
static constexpr float kTerrainHeightAmp = 10.0f;
static constexpr float kTerrainGroundOffset = 0.0f;

float TerrainHeightAt(float worldX, float freq, float amp, float groundOffset) 
{
    return groundOffset + amp * Perlin1D(worldX * freq);
}

struct TerrainDefinition
{
    float terrainFrequency = kTerrainHeightFreq;
    float terrainAmplitude = kTerrainHeightAmp;
    float groundOffset = kTerrainGroundOffset;
    float worldGroundY = 0.0f;
    Range<float> worldSpan = { 0.0f, 200.0f };
    int segmentCount = static_cast<int>(worldSpan.max - worldSpan.min);
};

Result<Entity> MakeTerrain(const TerrainDefinition& terrainDef, B2World& world)
{
    auto e = ECS::CreateEntity();
    assert(e.IsValid());

    auto& rigid = e.AddComponent(ComponentBuilder<RigidBody>{}
    .WithBodyParameters({
        .bodyType = B2Body::Type::Static,
        .position = { 0.0f, 0.0f }
    }).Build(world));

    auto& body = WriteAccessor<B2Body>{}(rigid.body);    
    if (!body.IsValid())
    {
        return MAKE_ERROR("Could not create body on chain entity");
    }

    assert(terrainDef.worldSpan.max < terrainDef.worldSpan.min);
    assert(terrainDef.segmentCount > 0);

    float startX = ToMeters(terrainDef.worldSpan.min);

    float segLength = (terrainDef.worldSpan.max - terrainDef.worldSpan.min) /
                       static_cast<float>(terrainDef.segmentCount);

    B2ChainDefinition chainDef{};
    chainDef.points.reserve(terrainDef.segmentCount);
    for (size_t i = 0; i < static_cast<size_t>(terrainDef.segmentCount); i++)
    {
        float x = startX + (static_cast<float>(i) * segLength);
        float y = TerrainHeightAt(x, 
            terrainDef.terrainFrequency, 
            terrainDef.terrainAmplitude,
            terrainDef.worldGroundY + terrainDef.groundOffset);

        chainDef.points.emplace_back(x, y);
    }
     
    chainDef.enableSensorEvents = false;
    chainDef.isLoop = false;

    TRY(body.AddChain(chainDef), chain);
    auto chainSegments = chain.GetSegments();

    auto rels = e.GetRelations();
    for (const auto& seg : chainSegments)
    {
        auto child = rels.AddChild();
        child.AddComponent<Transform>();
        
        auto& shape = WriteAccessor<B2Shape>{}(child.AddComponent<Collider>().shape);
        shape = seg;

        auto& profile = child.AddComponent<SpriteRenderableComponent>().profile;
        profile.debugDraw.collider.on = true;
    }

    return e;
}



} // game