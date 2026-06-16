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

struct UserSystemWithUpdateOperations
{
	static constexpr const char* kOutputMsgSetup = "Setup Operation";
	static constexpr const char* kOutputMsgInput = "Input Operation";
	static constexpr const char* kOutputMsgPresentation1 = "Presentation1 Operation";
	static constexpr const char* kOutputMsgPresentation2 = "Presentation2 Operation";

	explicit UserSystemWithUpdateOperations(std::string& shStr) : sharedStr(&shStr) {}

	std::string* sharedStr = nullptr;

	void SetupOperation(float) { if (sharedStr) { *sharedStr = kOutputMsgSetup; } }
	void InputOperation(float) { if (sharedStr) { *sharedStr = kOutputMsgInput; } }
	void PresentationOperation1(float) { if (sharedStr) { *sharedStr = kOutputMsgPresentation1; } }
	void PresentationOperation2(float) { if (sharedStr) { 
		*sharedStr += " " + std::string{kOutputMsgPresentation2};
	} }
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

TEST_CASE("Registering individual update operations on UserSystemScheduler", "[user][system]")
{
	UserSystemScheduler sysScheduler{};
	using UserSys = UserSystemWithUpdateOperations;

	STATIC_CHECK_FALSE(ImplementsSystemUpdate<UserSys>);

	std::string message;

	auto& sys = sysScheduler.RegisterSystem<UserSys>(message);
	CHECK(sysScheduler.IsSystemRegistered<UserSys>());

	sysScheduler.RegisterUpdateOperation<&UserSys::SetupOperation>(Phase::Setup);
	sysScheduler.RegisterUpdateOperation<&UserSys::InputOperation>(Phase::Input);
	sysScheduler.RegisterUpdateOperation<&UserSys::PresentationOperation1>(Phase::Presentation);
	sysScheduler.RegisterUpdateOperation<&UserSys::PresentationOperation2>(Phase::Presentation);

	sysScheduler.UpdateSystems(Phase::Setup, 0.0f);
	CHECK(message == UserSys::kOutputMsgSetup);

	sysScheduler.UpdateSystems(Phase::Input, 0.0f);
	CHECK(message == UserSys::kOutputMsgInput);

	sysScheduler.UpdateSystems(Phase::Presentation, 0.0f);

	const std::string expectedPresentationMsg = 
		std::string{UserSys::kOutputMsgPresentation1} + " " + 
		std::string{UserSys::kOutputMsgPresentation2};

	CHECK(message == expectedPresentationMsg);
}

TEST_CASE("Removing Systems", "[user][system]")
{
	UserSystemScheduler sysScheduler{};
	using UserSys = UserSystemWithUpdateOperations;

	std::string message;

	auto& sys = sysScheduler.RegisterSystem<UserSys>(message);
	CHECK(sysScheduler.IsSystemRegistered<UserSys>());

	sysScheduler.RegisterUpdateOperation<&UserSys::PresentationOperation1>(Phase::Presentation);
	sysScheduler.RegisterUpdateOperation<&UserSys::PresentationOperation2>(Phase::Presentation);

	sysScheduler.UpdateSystems(Phase::Presentation, 0.0f);

	const std::string expectedPresentationMsg =
		std::string{ UserSys::kOutputMsgPresentation1 } + " " +
		std::string{ UserSys::kOutputMsgPresentation2 };

	CHECK(message == expectedPresentationMsg);

	message.clear();

	const bool removed = sysScheduler.RemoveSystem<UserSys>();
	CHECK(removed);

	sysScheduler.UpdateSystems(Phase::Presentation, 0.0f);

	CHECK(message.empty());
}