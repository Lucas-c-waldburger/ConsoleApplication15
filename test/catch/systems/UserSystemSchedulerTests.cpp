#include "../CatchUtils.h"
#include "../../../user/UserSystemScheduler.h"

namespace {

struct UserSystemA
{
	UserSystemA() = default;
	explicit UserSystemA(int i) : x(i) {}

	void Update(float dt) { output = "updatedA"; }
	std::string output;
	int x;
};
struct UserSystemB
{
	UserSystemB() = default;
	explicit UserSystemB(std::unique_ptr<int> iPtr) : xPtr(std::move(iPtr)) {}

	void Update(float dt) { output = "updatedB"; }
	std::string output;
	std::unique_ptr<int> xPtr;
};
struct UserSystemC 
{ 
	static constexpr const char* kOutputMsg = "UserSystemC";
	std::string* sharedStr = nullptr; 
	size_t* sharedSizeT = nullptr;
	size_t updatedOrder = 0;
	void Update(float dt) 
	{ 
		if (sharedStr) 
		{ 
			*sharedStr = kOutputMsg; 
		} 
		if (sharedSizeT)
		{
			updatedOrder = *sharedSizeT;
			++*sharedSizeT;
		}
	}
};
struct UserSystemD 
{ 
	static constexpr const char* kOutputMsg = "UserSystemD";
	std::string* sharedStr = nullptr;
	size_t* sharedSizeT = nullptr;
	size_t updatedOrder = 0;
	void Update(float dt)
	{
		if (sharedStr)
		{
			*sharedStr = kOutputMsg;
		}
		if (sharedSizeT)
		{
			updatedOrder = *sharedSizeT;
			++*sharedSizeT;
		}
	}
};
struct UserSystemE 
{ 
	static constexpr const char* kOutputMsg = "UserSystemE";
	std::string* sharedStr = nullptr;
	size_t* sharedSizeT = nullptr;
	size_t updatedOrder = 0;
	void Update(float dt)
	{
		if (sharedStr)
		{
			*sharedStr = kOutputMsg;
		}
		if (sharedSizeT)
		{
			updatedOrder = *sharedSizeT;
			++*sharedSizeT;
		}
	}
};

} // unnamed

TEST_CASE("UserSystemScheduler Tests", "[user][system]")
{
	UserSystemScheduler sysScheduler{};

	// check scheduler concept
	STATIC_CHECK(ImplementsSystemUpdate<UserSystemA>);
	STATIC_CHECK(ImplementsSystemUpdate<UserSystemB>);
	class IllFormedUserSystem {};
	STATIC_CHECK_FALSE(ImplementsSystemUpdate<IllFormedUserSystem>);

	auto& sysA = sysScheduler.RegisterSystem<UserSystemA>(
		Phase::Setup, 69);
	CHECK(sysScheduler.IsSystemRegistered<UserSystemA>());
	CHECK(sysA.output.empty());
	CHECK(sysA.x == 69);

	auto& sysB = sysScheduler.RegisterSystem<UserSystemB>(
		Phase::Simulation, std::make_unique<int>(420));
	CHECK(sysB.output.empty());
	CHECK(sysB.xPtr != nullptr);
	CHECK(sysB.output.empty());

	// accidentally double register, shouldn't overrwrite existing system
	auto& sysAReRegistered = sysScheduler.RegisterSystem<UserSystemA>(
		Phase::Intent, 666);
	CHECK(sysScheduler.IsSystemRegistered<UserSystemA>());
	CHECK(sysA.output.empty());
	CHECK(sysA.x == 69);

	SECTION("Fire system updates for phase")
	{
		sysScheduler.UpdateSystems(Phase::Setup, 0.0f);
		CHECK(sysA.output == "updatedA");
		CHECK(sysB.output.empty());

		sysA.output.clear();
		sysScheduler.UpdateSystems(Phase::Simulation, 0.0f);
		CHECK(sysB.output == "updatedB");
		CHECK(sysA.output.empty());

		// fire empty phase, should not affect UserSystemA or UserSystemB
		sysB.output.clear();

		sysScheduler.UpdateSystems(Phase::Cleanup, 0.0f);
		CHECK(sysA.output.empty());
		CHECK(sysB.output.empty());
	}

	SECTION("Multiple systems with same phase update order")
	{
		std::string output;
		size_t order = 0;

		STATIC_CHECK(ImplementsSystemUpdate<UserSystemC>);
		STATIC_CHECK(ImplementsSystemUpdate<UserSystemD>);
		STATIC_CHECK(ImplementsSystemUpdate<UserSystemE>);

		auto& sysC = 
			sysScheduler.RegisterSystem<UserSystemC>(Phase::Presentation, 
				&output, &order);
		CHECK(sysScheduler.IsSystemRegistered<UserSystemC>());
		REQUIRE(sysC.sharedStr == &output);
		REQUIRE(sysC.sharedSizeT == &order);
		REQUIRE(sysC.updatedOrder == 0);

		auto& sysD =
			sysScheduler.RegisterSystem<UserSystemD>(Phase::Presentation,
				&output, &order);
		CHECK(sysScheduler.IsSystemRegistered<UserSystemD>());
		REQUIRE(sysD.sharedStr == &output);
		REQUIRE(sysD.sharedSizeT == &order);
		REQUIRE(sysD.updatedOrder == 0);

		auto& sysE =
			sysScheduler.RegisterSystem<UserSystemE>(Phase::Presentation,
				&output, &order);
		CHECK(sysScheduler.IsSystemRegistered<UserSystemE>());
		REQUIRE(sysE.sharedStr == &output);
		REQUIRE(sysE.sharedSizeT == &order);
		REQUIRE(sysE.updatedOrder == 0);

		sysScheduler.UpdateSystems(Phase::Presentation, 0.0f);
		CHECK(output == UserSystemE::kOutputMsg);
		CHECK(order == 3);

		CHECK(sysC.updatedOrder == 0);
		CHECK(sysD.updatedOrder == 1);
		CHECK(sysE.updatedOrder == 2);
	}
}