#include "../CatchMain.cpp"
#include "../../../ecs/EntityT.h"


TEST_CASE("Entity_t Tests", "[ecs]")
{
    SECTION("Is Entity_t Valid")
    {
        constexpr Entity_t validEntity = 0x0000000200000004; // Gen 2, Index 4
        
        CHECK(IsEntity_tValid(validEntity));
        CHECK_FALSE(IsEntity_tValid(kInvalidEntity));
    }

    SECTION("Decomposing Entity_t into EntityGeneration_t and EntityIndex_t")
    {
        constexpr Entity_t entity = 0x0000000100000003; // Gen 1, Index 3

        CHECK(IsEntity_tValid(entity));

        constexpr EntityGeneration_t expectedGeneration = 1;
        constexpr EntityIndex_t expectedIndex = 3;

        CHECK(GetEntity_tGeneration(entity) == expectedGeneration);
        CHECK(GetEntity_tIndex(entity) == expectedIndex);

        constexpr std::pair<EntityGeneration_t, EntityIndex_t> decomposed = 
            DecomposeEntity_t(entity);

        CHECK(decomposed.first == expectedGeneration);
        CHECK(decomposed.second == expectedIndex);
    }

    SECTION("Incrementing Entity_t Generation")
    {
        Entity_t entity = 0x0000000B00000005; // Gen 11, Index 5

        CHECK(IsEntity_tValid(entity));
        CHECK(GetEntity_tGeneration(entity) == 11);
        CHECK(GetEntity_tIndex(entity) == 5);

        entity = IncrementEntity_tGeneration(entity);

        CHECK(IsEntity_tValid(entity));
        CHECK(GetEntity_tGeneration(entity) == 12);
        CHECK(GetEntity_tIndex(entity) == 5);

        // make sure incrementing a max generation returns invalid
        Entity_t maxGenEntity = 0xFFFFFFFF00000007; // Gen 4'294'967'295, Index 7

        CHECK(GetEntity_tGeneration(maxGenEntity) == kMaxEntityGenerations);
        CHECK(IsEntity_tValid(maxGenEntity)); // (an entity at max gen is still valid)

        maxGenEntity = IncrementEntity_tGeneration(maxGenEntity);
        CHECK(maxGenEntity == kInvalidEntity);
        CHECK_FALSE(IsEntity_tValid(maxGenEntity));
    }
}