#include "../CatchUtils.h"
#include "../../../ecs/Ecs.h"
#include "../../Fixtures.h"
#include "../../../ecs/EntityEvents.h"
#include "../../../events/data/EventDataIncludes.h"

TEST_CASE("EntityEvents Tests", "[events]")
{
	auto fixtureOc = SceneFixture::GetInstance();
	REQUIRE_RESULT(fixtureOc);

	auto& fixture = fixtureOc.GetValue();
	REQUIRE(fixture);

	auto& bus = fixture->GetEventBus();

	auto e = ECS::CreateEntity();
	REQUIRE(e.IsValid());

	auto evs = e.GetEvents(bus);

	std::string evOutput1, evOutput2, evOutput3;

	SECTION("EventData-only callback")
	{
		CHECK(evOutput1.empty());
		CHECK_FALSE(e.HasComponent<SignalTokenStorage>());

		auto lamb = [&evOutput1](const events::GameLoopStepStart& ev) {
			evOutput1 = "fired";
		};

		STATIC_CHECK(ev_callback_sig<decltype(lamb)>::with_event_data_only_v);

		auto result = evs.OnEvent(std::move(lamb));
		REQUIRE_RESULT(result);

		REQUIRE(e.HasComponent<SignalTokenStorage>());
		CHECK(e.GetComponent<SignalTokenStorage>().signalTokens.size() == 1);

		bus.PushEvent(events::GameLoopStepStart{});
		bus.DispatchEvents();

		CHECK(evOutput1 == "fired");
	}
	SECTION("With non-const Entity callback")
	{
		CHECK_FALSE(e.HasComponent<SignalTokenStorage>());

		auto lamb = [](const events::GameLoopStepRender& ev, Entity& ent) {
			ent.AddComponent<Transform>().rotation = 125.0f;
		};

		STATIC_CHECK(ev_callback_sig<decltype(lamb)>::with_entity_v);

		STATIC_CHECK(SomeEventData<std::remove_cvref_t<
			typename func_traits<decltype(lamb)>::template arg_at<0>>>);

		auto result = evs.OnEvent(std::move(lamb));
		REQUIRE_RESULT(result);

		REQUIRE(e.HasComponent<SignalTokenStorage>());
		CHECK(e.GetComponent<SignalTokenStorage>().signalTokens.size() == 1);

		bus.PushEvent(events::GameLoopStepRender{});
		bus.DispatchEvents();

		REQUIRE(e.HasComponent<Transform>());
		CHECK(e.GetComponent<Transform>().rotation == 125.0f);
	}
	SECTION("With const Entity callback")
	{
		CHECK_FALSE(e.HasComponent<SignalTokenStorage>());
		CHECK(evOutput2.empty());

		auto lamb = [&evOutput2](const events::GameLoopStepEnd& ev, const Entity& ent) {
			evOutput2 = "fired";
		};

		STATIC_CHECK(ev_callback_sig<decltype(lamb)>::with_const_entity_v);

		auto result = evs.OnEvent(std::move(lamb));
		REQUIRE_RESULT(result);

		REQUIRE(e.HasComponent<SignalTokenStorage>());
		CHECK(e.GetComponent<SignalTokenStorage>().signalTokens.size() == 1);

		bus.PushEvent(events::GameLoopStepEnd{});
		bus.DispatchEvents();

		CHECK(evOutput2 == "fired");
	}
	SECTION("With non-const Components callback")
	{
		CHECK_FALSE(e.HasComponent<SignalTokenStorage>());

		e.AddComponent<Transform>();
		e.AddComponent<Tags>();

		auto lamb = [](const events::ContactCollisionBegin& ev, 
					   Transform& tf, Tags& tags) {
			tf.position = { 50.0f, -50.0f };
			tags.tags.insert("fired");
		};

		STATIC_CHECK(ev_callback_sig<decltype(lamb)>::with_components_v);

		auto result = evs.OnEvent(std::move(lamb));
		REQUIRE_RESULT(result);

		REQUIRE(e.HasComponent<SignalTokenStorage>());
		CHECK(e.GetComponent<SignalTokenStorage>().signalTokens.size() == 1);

		bus.PushEvent(events::ContactCollisionBegin{});
		bus.DispatchEvents();

		REQUIRE(e.HasComponent<Transform>());
		REQUIRE(e.HasComponent<Tags>());

		auto& tf = e.GetComponent<Transform>();
		auto& tags = e.GetComponent<Tags>();

		CHECK(tf.position == SDL_FPoint{ 50.0f, -50.0f });
		CHECK(tags.tags.size() == 1);
		CHECK(tags.tags.contains("fired"));
	}

	SECTION("InputEvents")
	{
		std::string outputA;
		std::string outputB;

		auto lambA = [&outputA](const events::GameControllerInput& ev) {
			outputA = "fired";
		};
		auto lambB = [&outputB](const events::GameControllerInput& ev, const Entity& ent) {
			CHECK(ent.IsValid());
			outputB = "fired";
		};

		STATIC_CHECK(ev_callback_sig<decltype(lambA)>::with_event_data_only_v);
		STATIC_CHECK(ev_callback_sig<decltype(lambB)>::with_const_entity_v);

		evs.OnInput(GameControllerInputSource::A, std::move(lambA));
		evs.OnInput(GameControllerInputSource::B, std::move(lambB));

		bus.PushEvent(events::GameControllerInput{
			.input = {.source = GameControllerInputSource::A } });
		bus.DispatchEvents();

		CHECK(outputA == "fired");
		CHECK(outputB.empty());
	}

	SECTION("UserEvents")
	{
		struct TestEvent1 { std::string message; };

		evs.OnEvent([&evOutput3](const TestEvent1& ev) { evOutput3 = ev.message; });

		bus.PushEvent(TestEvent1{ .message = "fired" });
		bus.DispatchEvents();

		CHECK(evOutput3 == "fired");
	}
}