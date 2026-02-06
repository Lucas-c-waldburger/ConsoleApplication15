#include "../CatchUtils.h"
#include "../../../ecs/Ecs.h"
#include "../../Fixtures.h"
#include "../../../ecs/EntityEvents.h"
#include "../../../events/data/EventDataIncludes.h"

TEST_CASE("EntityEvents Tests", "[ecs][events]")
{
	auto fixtureOc = SceneFixture::GetInstance();
	REQUIRE_RESULT(fixtureOc);

	auto& fixture = fixtureOc.GetValue();
	REQUIRE(fixture);

	auto& bus = fixture->GetEventBus();

	auto e = ECS::CreateEntity();
	REQUIRE(e.IsValid());

	auto evs = e.GetEvents(bus);

	SECTION("EventData-only callback")
	{
		std::string evOutput;

		CHECK_FALSE(e.HasComponent<SignalTokenStorage>());

		auto lamb = [&evOutput](const events::GameLoopStepStart& ev) {
			evOutput = "fired";
		};

		STATIC_CHECK(ev_callback_sig<decltype(lamb)>::with_event_data_only_v);

		auto result = evs.OnEvent(std::move(lamb));
		REQUIRE_RESULT(result);

		REQUIRE(e.HasComponent<SignalTokenStorage>());
		CHECK(e.GetComponent<SignalTokenStorage>().signalTokens.size() == 1);

		bus.PushEvent(events::GameLoopStepStart{});
		bus.DispatchEvents();

		CHECK(evOutput == "fired");
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
		std::string evOutput;

		CHECK_FALSE(e.HasComponent<SignalTokenStorage>());

		auto lamb = [&evOutput](const events::GameLoopStepEnd& ev, const Entity& ent) {
			evOutput = "fired";
		};

		STATIC_CHECK(ev_callback_sig<decltype(lamb)>::with_const_entity_v);

		auto result = evs.OnEvent(std::move(lamb));
		REQUIRE_RESULT(result);

		REQUIRE(e.HasComponent<SignalTokenStorage>());
		CHECK(e.GetComponent<SignalTokenStorage>().signalTokens.size() == 1);

		bus.PushEvent(events::GameLoopStepEnd{});
		bus.DispatchEvents();

		CHECK(evOutput == "fired");
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

		std::string evOutput;

		evs.OnEvent([&evOutput](const TestEvent1& ev) { evOutput = ev.message; });

		bus.PushEvent(TestEvent1{ .message = "fired" });
		bus.DispatchEvents();

		CHECK(evOutput == "fired");
	}
}

TEST_CASE("EntityParticipant Event Tests", "[ecs][events]")
{
	auto fixtureOc = SceneFixture::GetInstance();
	REQUIRE_RESULT(fixtureOc);

	auto& fixture = fixtureOc.GetValue();
	REQUIRE(fixture);

	auto& bus = fixture->GetEventBus();

	auto e = ECS::CreateEntity();
	REQUIRE(e.IsValid());

	auto evs = e.GetEvents(bus);

	SECTION("Relevant entity")
	{
		STATIC_CHECK(HasEntityParticipants<events::EntityPositionChanged>);

		std::string evOutput1, evOutput2, evOutput3;

		auto otherEntity1 = ECS::CreateEntity();
		REQUIRE(otherEntity1.IsValid());
		auto otherEntity2 = ECS::CreateEntity();
		REQUIRE(otherEntity2.IsValid());

		// no entity specified, will fire if ev involves this entity
		evs.OnEvent([&evOutput1](const events::EntityPositionChanged& ev,
								 const Entity& ent) {
			CHECK(ev.entity<0>() == ent.GetID());
			evOutput1 = "unspecified fired";
		});

		// other entity specified, will fire if ev involves that entity
		evs.OnEvent([&evOutput2, otherEntity1](const events::EntityPositionChanged& ev,
											   const Entity& ent) {
			CHECK(ev.entity<0>() == otherEntity1.GetID());
			evOutput2 = "specified fired";
		}, { .relevantEntity = otherEntity1.GetID() });

		// other entity specified, but event wont fire for it
		evs.OnEvent([&evOutput3](const events::EntityPositionChanged& ev,
								 const Entity& ent) {
			evOutput3 = "wont fire";
		}, { .relevantEntity = otherEntity2.GetID() });

		events::EntityPositionChanged ev1, ev2, ev3;
		ev1.entity<0>() = e.GetID();
		ev2.entity<0>() = otherEntity1.GetID();
		ev3.entity<0>() = e.GetID();

		bus.PushEvent(ev1);
		bus.PushEvent(ev2);
		bus.PushEvent(ev3);
		bus.DispatchEvents();

		CHECK(evOutput1 == "unspecified fired");
		CHECK(evOutput2 == "specified fired");
		CHECK(evOutput3.empty());
	}

	SECTION("Multi-entity participant event")
	{
		struct TestEvent : EntityParticipants<3> {};

		std::string evOutput1, evOutput2;

		auto otherEntity1 = ECS::CreateEntity();
		REQUIRE(otherEntity1.IsValid());
		auto otherEntity2 = ECS::CreateEntity();
		REQUIRE(otherEntity2.IsValid());
		auto otherEntity3 = ECS::CreateEntity();
		REQUIRE(otherEntity3.IsValid());

		TestEvent ev{};
		ev.entity<0>() = otherEntity1.GetID();
		ev.entity<1>() = otherEntity2.GetID();
		ev.entity<2>() = otherEntity3.GetID();

		evs.OnEvent([&evOutput1](const TestEvent& ev) {
			evOutput1 = "fired";
		}, { .relevantEntity = otherEntity2.GetID() });

		evs.OnEvent([&evOutput2](const TestEvent& ev) {
			evOutput2 = "wont fire";
		}, { .relevantEntity = e.GetID() });

		bus.PushEvent(ev);
		bus.DispatchEvents();

		CHECK(evOutput1 == "fired");
		CHECK(evOutput2.empty());
	}

	SECTION("Relevant game controller id")
	{
		STATIC_CHECK(SomeGameControllerEvent<events::GameControllerConnected>);
		STATIC_CHECK(SomeGameControllerEvent<events::GameControllerDisconnected>);
		STATIC_CHECK(SomeGameControllerEvent<events::GameControllerInput>);

		std::string evOutput1, evOutput2, evOutput3;

		e.AddComponent<GameControllerState>().joystickID = 5;

		evs.OnEvent([&evOutput1](const events::GameControllerConnected& ev, 
								 const Entity& e) {
			evOutput1 = "fired";
		});
		evs.OnEvent([&evOutput2](const events::GameControllerDisconnected& ev,
								 const Entity& e) {
			evOutput2 = "fired";
		}, { .relevantJoystickId = 5 });

		auto otherEntity1 = ECS::CreateEntity();
		REQUIRE(otherEntity1.IsValid());

		otherEntity1.AddComponent<GameControllerState>().joystickID = 3;

		evs.OnEvent([&evOutput3](const events::GameControllerConnected& ev,
			const Entity& e) {
				evOutput3 = "fired";
			}, { .relevantEntity = otherEntity1.GetID(), 
				 .relevantJoystickId = 3 });

		bus.PushEvent(events::GameControllerConnected{
			.joystickID = 5
		});
		bus.PushEvent(events::GameControllerDisconnected{
			.joystickID = 5
		});
		bus.PushEvent(events::GameControllerConnected{
			.joystickID = 3
		});

		bus.DispatchEvents();

		CHECK(evOutput1 == "fired");
		CHECK(evOutput2 == "fired");
		CHECK(evOutput3 == "fired");

		// check input event
		std::string evOutput4, evOutput5;

		auto otherEntity2 = ECS::CreateEntity();
		REQUIRE(otherEntity2.IsValid());

		otherEntity2.AddComponent<GameControllerState>().joystickID = 7;

		// unspecified joystickId will grab other entity's id automatically
		evs.OnInput(GameControllerInputSource::LeftStickAxis,
			[&evOutput4](const events::GameControllerInput& ev) {
				evOutput4 = "fired";
		}, { .relevantEntity = otherEntity2.GetID() });

		// mismatch wont
		evs.OnInput(GameControllerInputSource::LeftStickAxis,
			[&evOutput5](const events::GameControllerInput& ev) {
				evOutput5 = "wont fire";
		}, { .relevantEntity = otherEntity2.GetID(),
			 .relevantJoystickId = 99 });

		bus.PushEvent(events::GameControllerInput{
			.joystickID = 7,
			.input = { .source = GameControllerInputSource::LeftStickAxis }
		});

		bus.DispatchEvents();

		CHECK(evOutput4 == "fired");
		CHECK(evOutput5.empty());
	}
}