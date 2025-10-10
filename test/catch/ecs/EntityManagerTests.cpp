#include "../CatchMain.cpp"
#include "../../../ecs/EntityManager.h"


TEST_CASE("EntityManager Tests", "[ecs]")
{
    Entity_t entity = 0x0000000100000001; // Generation 1, Index 1
    CHECK(GetEntityGeneration(entity) == 1);
    CHECK(GetEntityIndex(entity) == 1);
    entity = IncrementEntityGeneration(entity);
    CHECK(GetEntityGeneration(entity) == 2);
    CHECK(GetEntityIndex(entity) == 1);

    EntityManager manager{};
    Entity_t e_1_0 = manager.CreateEntity();
    Entity_t e_1_1 = manager.CreateEntity();
    Entity_t e_1_2 = manager.CreateEntity();

    CHECK(GetEntityGeneration(e_1_0) == 1);
    CHECK(GetEntityIndex(e_1_0) == 0);
    CHECK(GetEntityGeneration(e_1_1) == 1);
    CHECK(GetEntityIndex(e_1_1) == 1);
    CHECK(GetEntityGeneration(e_1_2) == 1);
    CHECK(GetEntityIndex(e_1_2) == 2);

    CHECK(manager.IsEntityActive(e_1_0));
    CHECK(manager.IsEntityActive(e_1_1));
    CHECK(manager.IsEntityActive(e_1_2));

    auto activeEntities = manager.GetActiveEntities();
    REQUIRE(activeEntities.size() == 3);
    CHECK(activeEntities[0] == e_1_0);
    CHECK(activeEntities[1] == e_1_1);
    CHECK(activeEntities[2] == e_1_2);

    manager.DestroyEntity(e_1_0);
    CHECK_FALSE(manager.IsEntityActive(e_1_0));

    activeEntities = manager.GetActiveEntities();
    REQUIRE(activeEntities.size() == 2);
    CHECK(activeEntities[0] == e_1_2);
    CHECK(activeEntities[1] == e_1_1);

    Entity_t e_2_0 = manager.CreateEntity();
    CHECK(GetEntityGeneration(e_2_0) == 2);
    CHECK(GetEntityIndex(e_2_0) == 0);

    CHECK(manager.IsEntityActive(e_1_1));
    CHECK(manager.IsEntityActive(e_1_2));
    CHECK(manager.IsEntityActive(e_2_0));

    activeEntities = manager.GetActiveEntities();
    REQUIRE(activeEntities.size() == 3);
    CHECK(activeEntities[0] == e_1_2);
    CHECK(activeEntities[1] == e_1_1);
    CHECK(activeEntities[2] == e_2_0);
}