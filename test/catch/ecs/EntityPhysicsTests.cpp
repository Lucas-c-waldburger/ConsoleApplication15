#include "../CatchUtils.h"
#include "../../../ecs/EntityPhysics.h"


TEST_CASE("EntityPhysics Tests", "[ecs][x]")
{
	auto e = ECS::CreateEntity();
	REQUIRE(e.IsValid());

	auto world = B2World::Create(0, 9.8f);
	REQUIRE(world.IsValid());

	auto phys = e.GetPhysics(world);
	CHECK_FALSE(phys.HasBody());
	CHECK(phys.GetColliderCount() == 0);
	CHECK(phys.GetColliderShapes().empty());
	CHECK(phys.GetColliderEntities().empty());

	auto body = phys.AddBody(B2Body::Type::Dynamic, { 100.0f, 160.0f });
	REQUIRE(body.IsValid());
	CHECK(phys.HasBody());
	CHECK(body.GetBodyType() == B2Body::Type::Dynamic);
	CHECK(body.GetPosition() == SDL_FPoint{ 100.0f, 160.0f });

	REQUIRE(e.HasComponent<Transform>());
	CHECK(e.GetComponent<Transform>().position == SDL_FPoint{ 100.0f, 160.0f });

	REQUIRE(e.HasComponent<RigidBody>());
	CHECK(e.GetComponent<RigidBody>().body.GetData() == body);

	auto primaryColliderShape = phys.AddColliderBox({ 20.0f, 30.0f });
	REQUIRE(primaryColliderShape.IsValid());
	CHECK(primaryColliderShape.GetShapeType() == B2Shape::Type::Polygon);

	// should belong to entity itself
	REQUIRE(e.HasComponent<Collider>());
	CHECK(e.GetComponent<Collider>().shape.GetData() == primaryColliderShape);
	
	CHECK(phys.GetColliderCount() == 1);
	auto shapes = phys.GetColliderShapes();
	REQUIRE(shapes.size() == 1);
	CHECK(shapes.back() == primaryColliderShape);

	SECTION("Adding additional colliders creates them as child entities")
	{
		auto rels = e.GetRelations();
		CHECK_FALSE(rels.IsParent());

		auto secondaryColliderShape = phys.AddColliderCircle(50.4f);
		REQUIRE(secondaryColliderShape.IsValid());
		CHECK(secondaryColliderShape.GetShapeType() == B2Shape::Type::Circle);
		CHECK(phys.GetColliderCount() == 2);

		CHECK(rels.IsParent());
		auto children = rels.GetChildren();
		REQUIRE(children.size() == 1);

		const auto& child = *children.begin();
		REQUIRE(child.HasComponent<Transform>());
		REQUIRE(child.HasComponent<Collider>());
		CHECK_FALSE(child.HasComponent<RigidBody>());

		const auto& childShape = child.GetComponent<Collider>().shape.GetData();
		REQUIRE(childShape.IsValid());
		CHECK(childShape == secondaryColliderShape);

		auto newShapes = phys.GetColliderShapes();
		auto colliderEnts = phys.GetColliderEntities();
		REQUIRE(newShapes.size() == 2);
		REQUIRE(colliderEnts.size() == 2);

		CHECK(colliderEnts[0] == e);
		CHECK(colliderEnts[1] == child);

		CHECK(newShapes[0] == primaryColliderShape);
		CHECK(newShapes[1] == secondaryColliderShape);
	}
}