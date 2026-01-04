#include "../CatchUtils.h"
#include "../../../ecs/Ecs.h"

namespace {

struct UserCmpA { int value = 0; };
struct UserCmpB { std::string value; };
struct UserCmpC { std::unique_ptr<int> value; };

} // unnamed

TEST_CASE("UserComponentBridge Unit Tests", "[ecs]")
{
	UserComponentBridge bridge{};

	STATIC_CHECK(UserComponentTypeList::size >= 3);

	static constexpr size_t availableCount = UserComponentTypeList::size;
	CHECK(bridge.GetAvailableComponentCount() == availableCount);

	bool registeredA = bridge.RegisterComponentData<UserCmpA>();
	CHECK(registeredA);
	CHECK(bridge.IsComponentDataRegistered<UserCmpA>());
	CHECK(bridge.GetAvailableComponentCount() == availableCount - 1);
	CHECK(bridge.GetComponentDataSignature<UserCmpA>() == 
		UserComponent0::componentBit);

	bool registeredB = bridge.RegisterComponentData<UserCmpB>();
	CHECK(registeredB);
	CHECK(bridge.IsComponentDataRegistered<UserCmpB>());
	CHECK(bridge.GetAvailableComponentCount() == availableCount - 2);
	CHECK(bridge.GetComponentDataSignature<UserCmpB>() == 
		UserComponent1::componentBit);

	bool registeredC = bridge.RegisterComponentData<UserCmpC>();
	CHECK(registeredC);
	CHECK(bridge.IsComponentDataRegistered<UserCmpC>());
	CHECK(bridge.GetAvailableComponentCount() == availableCount - 3);
	CHECK(bridge.GetComponentDataSignature<UserCmpC>() == 
		UserComponent2::componentBit);
}

TEST_CASE("UserComponentBridge Integration Tests", "[ecs]")
{
	STATIC_CHECK(UserComponentTypeList::size >= 3);

	SECTION("Entity API")
	{
		auto entity = ECS::CreateEntity();
		REQUIRE(entity.IsValid());

		entity.AddComponent<UserCmpA>();

		CHECK(ECS::IsComponentRegistered<Transform>());
		CHECK(ECS::IsComponentRegistered<UserCmpA>());
		CHECK_FALSE(ECS::IsComponentRegistered<UserCmpB>());

		CHECK(entity.HasComponent<UserCmpA>());

		entity.GetComponent<UserCmpA>().value = 99;
		CHECK(entity.GetComponent<UserCmpA>().value == 99);

		entity.RemoveComponent<UserCmpA>();
		CHECK_FALSE(entity.HasComponent<UserCmpA>());

		entity.AddComponent<UserCmpB>();
		entity.AddComponent<UserCmpC>();

		CHECK(ECS::IsComponentRegistered<UserCmpB>());
		CHECK(ECS::IsComponentRegistered<UserCmpC>());

		CHECK(entity.HasComponent<UserCmpB>());
		CHECK(entity.HasComponent<UserCmpC>());

		entity.GetComponent<UserCmpB>().value = "user component b";
		entity.GetComponent<UserCmpC>().value = std::make_unique<int>(69);

		CHECK(entity.GetComponent<UserCmpB>().value == "user component b");
		CHECK(entity.GetComponent<UserCmpC>().value);
		CHECK(*entity.GetComponent<UserCmpC>().value == 69);

		entity.RemoveComponent<UserCmpB>();
		entity.RemoveComponent<UserCmpC>();
		CHECK_FALSE(entity.HasComponent<UserCmpB>());
		CHECK_FALSE(entity.HasComponent<UserCmpC>());
	}

	SECTION("Get entities ECS API")
	{
		auto e1 = ECS::CreateEntity();
		auto e2 = ECS::CreateEntity();
		auto e3 = ECS::CreateEntity();
		auto e4 = ECS::CreateEntity();
		auto e5 = ECS::CreateEntity();

		e1.AddComponent<Transform>();
		e1.AddComponent<UserCmpA>();

		e2.AddComponent<Transform>();
		e2.AddComponent<UserCmpB>();

		e3.AddComponent<CameraTarget>();
		e3.AddComponent<UserCmpC>();

		e4.AddComponent<UserCmpA>();
		e4.AddComponent<UserCmpC>();

		e5.AddComponent<CameraTarget>();

		auto eraseIfContains = [](const Entity& e, std::vector<Entity>& eVec) {
			auto it = std::find(eVec.begin(), eVec.end(), e);
			if (it == eVec.end())
			{
				return false;
			}
			eVec.erase(it);
			return true;
		};

		auto eVec1 = ECS::GetAllEntitiesWith<UserCmpA>();
		REQUIRE(eVec1.size() == 2);
		CHECK(eraseIfContains(e1, eVec1));
		CHECK(eraseIfContains(e4, eVec1));

		auto eVec2 = ECS::GetAllEntitiesWithAny<UserCmpA, Transform>();
		REQUIRE(eVec2.size() == 3);
		CHECK(eraseIfContains(e1, eVec2));
		CHECK(eraseIfContains(e2, eVec2));
		CHECK(eraseIfContains(e4, eVec2));

		auto eVec3 = ECS::GetAllEntitiesWith<Any<UserCmpB, UserCmpC>>();
		REQUIRE(eVec3.size() == 3);
		CHECK(eraseIfContains(e2, eVec3));
		CHECK(eraseIfContains(e3, eVec3));
		CHECK(eraseIfContains(e4, eVec3));

		auto eVec4 = ECS::GetAllEntitiesWithOnly<CameraTarget>();
		REQUIRE(eVec4.size() == 1);
		CHECK(eVec4.front() == e5);

		auto eVec5 = ECS::GetAllEntitiesWith<CameraTarget, Exclude<UserCmpC>>();
		REQUIRE(eVec5.size() == 1);
		CHECK(eVec5.front() == e5);
	}
}