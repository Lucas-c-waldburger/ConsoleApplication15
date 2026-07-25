#include "../CatchUtils.h"
#include "../../../ecs/Ecs.h"
#include "../../../physics/B2World.h"
#include "../../../physics/B2Shape.h"
#include "../../../components/builder/RigidBodyComponentBuilder.h"
#include "../../../components/builder/ColliderComponentBuilder.h"
#include "../../../systems/PhysicsSystem.h"
#include "../../../systems/ScriptSystem.h"
#include "../../../file/FilePathUtility.h"

TEST_CASE("Entity destroyed signal - PhysicsSystem", "[ecs][sys][phys]")
{
	auto world = B2World::Create(0, 9.8f);
	REQUIRE(world.IsValid());

	auto bodyEnt = ECS::CreateEntity();
	REQUIRE(bodyEnt.IsValid());
	bodyEnt.AddComponent(Transform{});

	auto& rb = bodyEnt.AddComponent(ComponentBuilder<RigidBody>{}.Build(world));
	REQUIRE(rb.body.GetData().IsValid());
	const auto& body = rb.body.GetData();

	CHECK(body.GetShapeCount() == 0);

	auto& selfCol = bodyEnt.AddComponent(ComponentBuilder<Collider>{}
	.WithShapeParameters(B2ShapeParameters{ .shapeType = B2Shape::Type::Polygon,
											.dimensions = Dimensions{ 34.5f, 23.6f }})
	.Build(rb.body));
	REQUIRE(selfCol.shape.GetData().IsValid());

	CHECK(body.GetShapeCount() == 1);

	auto rels = bodyEnt.GetRelations();

	auto childEnt1 = ECS::CreateEntity();
	REQUIRE(childEnt1.IsValid());
	childEnt1.AddComponent(Transform{});

	auto& chCol1 = childEnt1.AddComponent(ComponentBuilder<Collider>{}
	.WithShapeParameters(B2ShapeParameters{ .shapeType = B2Shape::Type::Polygon,
											.hull = std::vector<SDL_FPoint>{
												{35.6f, 32.3f}, {52.6f, 55.6f}, {56.4f, 15.3f}}})
	.Build(rb.body));
	REQUIRE(chCol1.shape.GetData().IsValid());

	CHECK(body.GetShapeCount() == 2);

	auto childEnt2 = ECS::CreateEntity();
	REQUIRE(childEnt2.IsValid());
	childEnt2.AddComponent(Transform{});

	auto& chCol2 = childEnt2.AddComponent(ComponentBuilder<Collider>{}
	.WithShapeParameters(B2ShapeParameters{ .shapeType = B2Shape::Type::Circle,
											.radius = 24.5 })
	.Build(rb.body));
	REQUIRE(chCol2.shape.GetData().IsValid());

	CHECK(body.GetShapeCount() == 3);


	PhysicsSystem phsSys{};

	auto ch2ShapeHandle = chCol2.shape.GetData().GetHandle();
	childEnt2.Destroy();

	CHECK_FALSE(childEnt2.IsValid());
	CHECK_FALSE(ch2ShapeHandle.IsValid());
	CHECK(body.IsValid());
	CHECK(body.GetShapeCount() == 2);

	auto ch1ShapeHandle = chCol1.shape.GetData().GetHandle();
	childEnt1.Destroy();

	CHECK_FALSE(childEnt1.IsValid());
	CHECK_FALSE(ch1ShapeHandle.IsValid());
	CHECK(body.IsValid());
	CHECK(body.GetShapeCount() == 1);

	auto entBodyHandle = body.GetHandle();
	auto entShapeHandle = selfCol.shape.GetData().GetHandle();

	bodyEnt.Destroy();
	CHECK_FALSE(bodyEnt.IsValid());
	CHECK_FALSE(body.IsValid());
	CHECK_FALSE(entBodyHandle.IsValid());
	CHECK_FALSE(entShapeHandle.IsValid());
}

TEST_CASE("Component removed signal - PhysicsSystem", "[ecs][sys][phys]")
{
	auto world = B2World::Create(0, 9.8f);
	REQUIRE(world.IsValid());

	auto bodyEnt = ECS::CreateEntity();
	REQUIRE(bodyEnt.IsValid());
	bodyEnt.AddComponent(Transform{});

	auto& rb = bodyEnt.AddComponent(ComponentBuilder<RigidBody>{}.Build(world));
	REQUIRE(rb.body.GetData().IsValid());
	const auto& body = rb.body.GetData();

	CHECK(body.GetShapeCount() == 0);

	auto& selfCol = bodyEnt.AddComponent(ComponentBuilder<Collider>{}
	.WithShapeParameters(B2ShapeParameters{ .shapeType = B2Shape::Type::Polygon,
											.dimensions = Dimensions{ 34.5f, 23.6f } })
		.Build(rb.body));
	REQUIRE(selfCol.shape.GetData().IsValid());

	CHECK(body.GetShapeCount() == 1);

	auto rels = bodyEnt.GetRelations();

	auto childEnt1 = ECS::CreateEntity();
	REQUIRE(childEnt1.IsValid());
	childEnt1.AddComponent(Transform{});

	auto& chCol1 = childEnt1.AddComponent(ComponentBuilder<Collider>{}
	.WithShapeParameters(B2ShapeParameters{ .shapeType = B2Shape::Type::Polygon,
											.hull = std::vector<SDL_FPoint>{
												{35.6f, 32.3f}, {52.6f, 55.6f}, {56.4f, 15.3f}} })
												.Build(rb.body));
	REQUIRE(chCol1.shape.GetData().IsValid());

	CHECK(body.GetShapeCount() == 2);

	auto childEnt2 = ECS::CreateEntity();
	REQUIRE(childEnt2.IsValid());
	childEnt2.AddComponent(Transform{});

	auto& chCol2 = childEnt2.AddComponent(ComponentBuilder<Collider>{}
	.WithShapeParameters(B2ShapeParameters{ .shapeType = B2Shape::Type::Circle,
											.radius = 24.5 })
		.Build(rb.body));
	REQUIRE(chCol2.shape.GetData().IsValid());

	CHECK(body.GetShapeCount() == 3);


	PhysicsSystem phsSys{};

	auto ch2ShapeHandle = chCol2.shape.GetData().GetHandle();
	childEnt2.RemoveComponent<Collider>();

	CHECK(childEnt2.IsValid());
	CHECK_FALSE(childEnt2.HasComponent<Collider>());
	CHECK_FALSE(ch2ShapeHandle.IsValid());
	CHECK(body.IsValid());
	CHECK(body.GetShapeCount() == 2);

	auto ch1ShapeHandle = chCol1.shape.GetData().GetHandle();
	childEnt1.RemoveComponent<Collider>();

	CHECK(childEnt1.IsValid());
	CHECK_FALSE(childEnt1.HasComponent<Collider>());
	CHECK_FALSE(ch1ShapeHandle.IsValid());
	CHECK(body.IsValid());
	CHECK(body.GetShapeCount() == 1);

	auto entBodyHandle = body.GetHandle();
	auto entShapeHandle = selfCol.shape.GetData().GetHandle();
	 
	bodyEnt.RemoveComponent<RigidBody>();

	CHECK(bodyEnt.IsValid());
	CHECK_FALSE(bodyEnt.HasComponent<RigidBody>());
	CHECK(bodyEnt.HasComponent<Collider>()); // still has collider, just not valid
	CHECK_FALSE(bodyEnt.GetComponent<Collider>().shape.GetData().IsValid());

	CHECK_FALSE(body.IsValid());
	CHECK_FALSE(entBodyHandle.IsValid());
	CHECK_FALSE(entShapeHandle.IsValid());
}

TEST_CASE("Entity destroyed signal - ScriptSystem", "[ecs][sys][phys]")
{
	static constexpr auto makeScriptPath = [](const auto& filename) {
		auto path = FilePathUtility::GetRootPath() /
			std::filesystem::path("test/catch/test_scripts") / std::filesystem::path(filename);

		REQUIRE(std::filesystem::exists(path));
		return path.string();
	};

	auto scriptPath = makeScriptPath("entity_test.lua");


}