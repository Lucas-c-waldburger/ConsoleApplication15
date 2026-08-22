#pragma once
#include <cmath>
#include <random>
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

inline float SmoothStep(float t) 
{
	return t * t * (3.0f - 2.0f * t);
}

inline float Perlin1D(float x)
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

inline float TerrainHeightAt(float worldX, float freq, float amp, float groundOffset)
{
    return groundOffset + amp * Perlin1D(worldX * freq);
}

inline float HillHeightAt(float x, float start, float width, float height)
{
    float t = (x - start) / width;   // 0 -> 1
    return height * 0.5f * (1.0f - std::cos(t * 2.0f * static_cast<float>(M_PI)));
}

struct TerrainDefinition
{
    float terrainFrequency = kTerrainHeightFreq;
    float terrainAmplitude = kTerrainHeightAmp;
    float groundOffset = kTerrainGroundOffset;
    float worldGroundY = 0.0f;
    Range<float> worldSpan = { 0.0f, 1000.0f };
    int segmentCount = static_cast<int>(worldSpan.max - worldSpan.min);
};

struct HillDefinition
{
    std::vector<float> startXs;
    std::vector<float> hillWidths;
    std::vector<float> hillHeights;

    void push_back(float x, float w, float h)
    {
        startXs.push_back(x);
        hillWidths.push_back(w);
        hillHeights.push_back(h);
    }
};

//inline Result<Entity> MakeTerrain(const TerrainDefinition& terrainDef, 
//                                  const HillDefinition& hillDef,
//                                  B2World& world)
//{
//    assert(hillDef.startXs.size() > 0 &&
//        hillDef.startXs.size() == hillDef.hillWidths.size() &&
//        hillDef.startXs.size() == hillDef.hillHeights.size());
//
//    auto e = ECS::CreateEntity();
//    assert(e.IsValid());
//
//    auto& rigid = e.AddComponent(ComponentBuilder<RigidBody>{}
//    .WithBodyParameters({
//        .bodyType = B2Body::Type::Static,
//        .position = { 0.0f, 0.0f }
//    }).Build(world));
//
//    auto& body = WriteAccessor<B2Body>{}(rigid.body);    
//    if (!body.IsValid())
//    {
//        return MAKE_ERROR("Could not create body on chain entity");
//    }
//
//    assert(terrainDef.worldSpan.max > terrainDef.worldSpan.min);
//    assert(terrainDef.segmentCount > 0);
//
//    float startX = ToMeters(terrainDef.worldSpan.min);
//
//    float terrainSpanDiff = terrainDef.worldSpan.max - terrainDef.worldSpan.min;
//    float segLength = terrainSpanDiff / static_cast<float>(terrainDef.segmentCount);
//
//    B2ChainDefinition chainDef{};
//    chainDef.enableSensorEvents = false;
//    chainDef.isLoop = false;
//    chainDef.points.reserve(terrainDef.segmentCount);
//    chainDef.materials.emplace_back();
//
//    size_t hillIdx = 0;
//    const size_t maxHillIdx = hillDef.startXs.size() - 1;
//    bool makingHill = false;
//    bool newHill = false;
//    float hillHeightMod = 0.0f;
//    float hillPeak = 0.0f;
//
//    for (size_t i = 0; i < static_cast<size_t>(terrainDef.segmentCount); i++)
//    {
//        float x = startX + (static_cast<float>(i) * segLength);
//        float y = 0.0f;
//
//        // making or about to start hill
//        if (hillIdx <= maxHillIdx && x >= hillDef.startXs[hillIdx])
//        {
//
//            makingHill = true;
//            // at or exceeded hill width
//            if (x >= hillDef.startXs[hillIdx] + hillDef.hillWidths[hillIdx])
//            {
//                // move to next hill index if not last
//                ++hillIdx;
//            }
//            if (hillIdx > maxHillIdx)
//            {
//                makingHill = false;
//            }
//            //if (hillIdx <= maxHillIdx)
//            //{
//            //    y = HillHeightAt(x,
//            //        hillDef.startXs[hillIdx],
//            //        hillDef.hillWidths[hillIdx],
//            //        hillDef.hillHeights[hillIdx]) - y;
//            //}
//            //else
//            //{
//            //    makingHill = false;
//            //}
//        }
//        else
//        {
//            makingHill = false;
//        }
//        if (!makingHill)
//        {
//            y = TerrainHeightAt(x,
//                terrainDef.terrainFrequency,
//                terrainDef.terrainAmplitude,
//                terrainDef.worldGroundY + terrainDef.groundOffset);
//        }
//
//        chainDef.points.emplace_back(x, y);
//    }  
//     
//    TRY(body.AddChain(chainDef), chain); 
//    auto chainSegments = chain.GetSegments();
//     
//    auto rels = e.GetRelations();
//    for (const auto& seg : chainSegments)
//    {
//        auto child = rels.AddChild();
//        child.AddComponent<Transform>();
//        
//        auto& shape = WriteAccessor<B2Shape>{}(child.AddComponent<Collider>().shape);
//        shape = seg;
//
//        auto& profile = child.AddComponent<SpriteRenderableComponent>().profile;
//        profile.debugDraw.collider.on = true;
//    }
//
//    return e;
//}



inline std::vector<SDL_FPoint> SmoothHillyTerrain(
    const std::vector<SDL_FPoint>& input,
    float smoothingRadius,     // world units (hill width)
    float noiseAmplitude,       // vertical variation
    float noiseFrequency,       // noise cycles per world unit
    uint32_t seed = 1337
)
{
    const int n = static_cast<int>(input.size());
    if (n < 3)
    {
        return input;
    }

    // ------------------------------------------------------------
    // 1) Copy input X/Y (X stays fixed forever)
    // ------------------------------------------------------------
    std::vector<float> x(n), y(n);
    for (int i = 0; i < n; ++i) 
    {
        x[i] = input[i].x;
        y[i] = input[i].y;
    }

    // ------------------------------------------------------------
    // 2) Build cumulative arc-length (handles variable spacing)
    // ------------------------------------------------------------
    std::vector<float> s(n);
    s[0] = 0.f;

    for (int i = 1; i < n; ++i) 
    {
        float dx = x[i] - x[i - 1];
        float dy = y[i] - y[i - 1];
        s[i] = s[i - 1] + std::sqrt(dx * dx + dy * dy);
    }

    // ------------------------------------------------------------
    // 3) Compute slopes between points
    // ------------------------------------------------------------
    std::vector<float> slope(n - 1);
    for (int i = 0; i < n - 1; ++i) 
    {
        float ds = s[i + 1] - s[i];
        slope[i] = (y[i + 1] - y[i]) / ds;
    }

    // ------------------------------------------------------------
    // 4) Distance-weighted smoothing of slopes (with variation)
    // ------------------------------------------------------------
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> jitter(0.9f, 1.1f);

    std::vector<float> smoothSlope(n - 1);

    for (int i = 0; i < n - 1; ++i) 
    {
        float si = (s[i] + s[i + 1]) * 0.5f;

        float sum = 0.f;
        float wsum = 0.f;

        for (int j = 0; j < n - 1; ++j) 
        {
            float sj = (s[j] + s[j + 1]) * 0.5f;
            float d = std::abs(sj - si);

            if (d > smoothingRadius)
            {
                continue;
            }

            float baseWeight = 1.f - (d / smoothingRadius);
            float w = baseWeight * jitter(rng);

            sum += slope[j] * w;
            wsum += w;
        }

        smoothSlope[i] = sum / wsum;
    }

    // ------------------------------------------------------------
    // 5) Reintegrate heights (endpoints stay fixed)
    // ------------------------------------------------------------
    std::vector<float> yOut(n);
    yOut[0] = y[0];

    for (int i = 1; i < n; ++i) 
    {
        float ds = s[i] - s[i - 1];
        yOut[i] = yOut[i - 1] + smoothSlope[i - 1] * ds;
    }

    // Hard-lock the final endpoint
    yOut[n - 1] = y[n - 1];

    // ------------------------------------------------------------
    // 6) Add low-frequency variation along terrain normal
    // ------------------------------------------------------------
    for (int i = 1; i < n - 1; ++i) 
    {
        float dx = x[i + 1] - x[i - 1];
        float dy = yOut[i + 1] - yOut[i - 1];

        float len = std::sqrt(dx * dx + dy * dy);
        if (len <= 0.f)
        {
            continue;
        }

        // Unit normal
        float nx = -dy / len;
        float ny = dx / len;

        // Simple continuous noise (can swap for Perlin later)
        float t = s[i] * noiseFrequency;
        float noise = std::sin(t * 6.2831853f + seed * 0.01f);

        yOut[i] += ny * noise * noiseAmplitude;
    }

    // ------------------------------------------------------------
    // 7) Rebuild output points
    // ------------------------------------------------------------
    std::vector<SDL_FPoint> output(n);
    for (int i = 0; i < n; ++i) {
        output[i] = SDL_FPoint{ x[i], yOut[i] };
    }

    return output;
}

inline const std::vector<SDL_FPoint> testTerrain = {
    {   0.f, 300.f },
    {  40.f, 290.f },
    {  85.f, 310.f },
    { 130.f, 305.f },
    { 170.f, 295.f },

    { 210.f, 280.f },
    { 255.f, 270.f },
    { 300.f, 265.f },

    { 360.f, 270.f },
    { 410.f, 285.f },
    { 470.f, 300.f },

    { 520.f, 305.f },
    { 560.f, 310.f },
    { 600.f, 310.f }, // plateau

    { 650.f, 300.f },
    { 710.f, 280.f },
    { 760.f, 260.f },

    { 820.f, 270.f },
    { 880.f, 290.f },
    { 940.f, 300.f },
    {1000.f, 300.f }
};

inline std::vector<SDL_FPoint> ResampleByDistance(
    const std::vector<SDL_FPoint>& input,
    float targetSpacing   // pixels between points
)
{
    if (input.size() < 2)
    {
        return input;
    }

    std::vector<SDL_FPoint> out;
    out.reserve(static_cast<size_t>(
        (input.back().x - input.front().x) / targetSpacing + 2));

    out.push_back(input.front());

    float remaining = targetSpacing;

    for (size_t i = 1; i < input.size(); ++i)
    {
        SDL_FPoint a = input[i - 1];
        SDL_FPoint b = input[i];

        float dx = b.x - a.x;
        float dy = b.y - a.y;
        float segLen = std::sqrt(dx * dx + dy * dy);

        while (segLen >= remaining) 
        {
            float t = remaining / segLen;

            SDL_FPoint p{
                a.x + dx * t,
                a.y + dy * t
            };

            out.push_back(p);

            a = p;
            dx = b.x - a.x;
            dy = b.y - a.y;
            segLen = std::sqrt(dx * dx + dy * dy);

            remaining = targetSpacing;
        }

        remaining -= segLen;
    }

    out.push_back(input.back());
    return out;
}

inline Result<Entity> MakeTerrain(const std::vector<SDL_FPoint>& input,
                                  B2World& world,
                                  float smoothingRadius,     
                                  float noiseAmplitude,      
                                  float noiseFrequency,      
                                  float resampleSpacing)
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

    B2ChainDefinition chainDef{};
    chainDef.enableSensorEvents = false;
    chainDef.isLoop = false;
    chainDef.materials.emplace_back();

    //auto pointsInMeters = input | std::views::transform([](const auto& p) {
    //    return SDL_FPoint{ ToMeters(p.x), ToMeters(p.y) };
    //    }) | std::ranges::to<std::vector>();

    auto dense = ResampleByDistance(input, resampleSpacing);
    chainDef.points = SmoothHillyTerrain(dense,
        smoothingRadius, noiseAmplitude, noiseFrequency);

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

template <typename T> requires std::is_default_constructible_v<T>
struct Dirty
{
    using value_type = T;

    Dirty() = default;
    ~Dirty() = default;
    Dirty(const Dirty&) = default;
    Dirty(Dirty&&) noexcept = default;
    Dirty& operator=(const Dirty&) = default;
    Dirty& operator=(Dirty&&) noexcept = default;
    
    template <typename...Args> requires std::constructible_from<T, Args...>
    explicit Dirty(Args&&...args) : value_(std::forward<Args>(args)...) {}

    template <typename U> requires std::is_assignable_v<T, U>
    Dirty& operator=(U&& u)
    {
        value_ = std::forward<U>(u);
        dirty_ = true;

        return *this;
    }

    T& operator*() { return get_value(); }
    const T& operator*() const { return get_value(); }

    T& get_value() 
    { 
        dirty_ = true;
        return value_; 
    }
    const T& get_value() const { return value_; }

    bool is_dirty() const { return dirty_; }

    void mark_dirty() { dirty_ = true; }
    void mark_clean() { dirty_ = false; }

private:
    T value_{};
    bool dirty_ = true;
};


} // game