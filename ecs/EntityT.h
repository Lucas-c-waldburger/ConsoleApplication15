#pragma once
#include <cstdint>
#include <utility>

// TYPES //
using Entity_t = uint64_t;   
using EntityIndex_t = uint32_t;        // Low 32 bits
using EntityGeneration_t = uint32_t;   // High 32 bits

// CONSTANTS //
inline constexpr Entity_t           kMaxEntities = 500;
inline constexpr EntityIndex_t      kMaxEntityIndex = kMaxEntities - 1;

inline constexpr EntityGeneration_t kMaxEntityGenerations = 0xFFFFFFFFu;

inline constexpr Entity_t           kInvalidEntity = 0xFFFFFFFFFFFFFFFFull;
inline constexpr EntityGeneration_t kInvalidEntityGen = 0; // First valid gen is 1

static_assert(kMaxEntities < 0xFFFFFFFFu, "kMaxEntities must fit in 32 bits");

// HELPERS //
inline constexpr EntityGeneration_t GetEntityGeneration(Entity_t entity)
{
    return static_cast<EntityGeneration_t>(entity >> 32);
}

inline constexpr EntityIndex_t GetEntityIndex(Entity_t entity)
{
    return static_cast<EntityIndex_t>(entity & 0xFFFFFFFFull);
}

inline constexpr std::pair<EntityGeneration_t, EntityIndex_t>
DecomposeEntity(Entity_t entity)
{
    return { GetEntityGeneration(entity), GetEntityIndex(entity) };
}

inline constexpr Entity_t IncrementEntityGeneration(Entity_t entity)
{
    return (static_cast<Entity_t>(GetEntityGeneration(entity) + 1) << 32)
        | GetEntityIndex(entity);
}

inline constexpr bool IsEntityValid(Entity_t entity)
{
    const auto [generation, index] = DecomposeEntity(entity);
    return index <= kMaxEntityIndex && generation <= kMaxEntityGenerations;
}