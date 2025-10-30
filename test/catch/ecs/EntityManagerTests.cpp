#include "../CatchUtils.h"
#include "../../../ecs/EntityManager.h"


TEST_CASE("EntityManager Tests", "[ecs]")
{
    EntityManager manager{};

    // creating entities
    Entity_t e_1_0 = manager.CreateEntity();
    Entity_t e_1_1 = manager.CreateEntity();
    Entity_t e_1_2 = manager.CreateEntity();

    CHECK(GetEntity_tGeneration(e_1_0) == 1);
    CHECK(GetEntity_tIndex(e_1_0) == 0);
    CHECK(GetEntity_tGeneration(e_1_1) == 1);
    CHECK(GetEntity_tIndex(e_1_1) == 1);
    CHECK(GetEntity_tGeneration(e_1_2) == 1);
    CHECK(GetEntity_tIndex(e_1_2) == 2);

    CHECK(manager.IsEntityActive(e_1_0));
    CHECK(manager.IsEntityActive(e_1_1));
    CHECK(manager.IsEntityActive(e_1_2));

    auto activeEntities = manager.GetActiveEntities();
    REQUIRE(activeEntities.size() == 3);
    CHECK(activeEntities[0] == e_1_0);
    CHECK(activeEntities[1] == e_1_1);
    CHECK(activeEntities[2] == e_1_2);

    // destroying entities
    manager.DestroyEntity(e_1_0);
    CHECK_FALSE(manager.IsEntityActive(e_1_0));

    activeEntities = manager.GetActiveEntities();
    REQUIRE(activeEntities.size() == 2);
    CHECK(activeEntities[0] == e_1_2);
    CHECK(activeEntities[1] == e_1_1);

    // creating new entity that reuses a destroyed entity's index
    Entity_t e_2_0 = manager.CreateEntity();
    CHECK(GetEntity_tGeneration(e_2_0) == 2);
    CHECK(GetEntity_tIndex(e_2_0) == 0);

    CHECK(manager.IsEntityActive(e_1_1));
    CHECK(manager.IsEntityActive(e_1_2));
    CHECK(manager.IsEntityActive(e_2_0));

    activeEntities = manager.GetActiveEntities();
    REQUIRE(activeEntities.size() == 3);
    CHECK(activeEntities[0] == e_1_2);
    CHECK(activeEntities[1] == e_1_1);
    CHECK(activeEntities[2] == e_2_0);
}