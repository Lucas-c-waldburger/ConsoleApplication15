#include "../CatchUtils.h"
#include "../../../events/EventBus2.h"

namespace ev = events;

struct EventTestFixture
{
	std::vector<SignalToken> signalTokens;
	std::array<uint32_t, EventDataTypeList::size> eventCallCounts;

	template <SomeEventData T>
	auto MakeEventCallCountLambda()
	{
		return [this](const T&) { ++eventCallCounts[T::eventType]; };
	}
};

TEST_CASE("EventBus stores pushed events", "[events]")
{
	EventBus2 bus{};
	EventTestFixture fixture{};

	bus.PushEvent(ev::EntityCreated{});
	bus.PushEvent(ev::EntityCreated{});
	bus.PushEvent(ev::EntityDestroyed{});
	bus.PushEvent(ev::EntityCreated{});
	bus.PushEvent(ev::EntityDestroyed{});
	bus.PushEvent(ev::TimerFired{});

	CHECK(bus.PeekEvents<ev::EntityCreated>().size() == 3);
	CHECK(bus.PeekEvents<ev::EntityDestroyed>().size() == 2);
	CHECK(bus.PeekEvents<ev::TimerFired>().size() == 1);
}

TEST_CASE("EventBus removes events on dispatch", "[events]")
{
	EventBus2 bus{};
	EventTestFixture fixture{};

	bus.PushEvent(ev::EntityCreated{});
	bus.PushEvent(ev::EntityCreated{});
	bus.PushEvent(ev::EntityCreated{});

	CHECK(bus.PeekEvents<ev::EntityCreated>().size() == 3);

	bus.DispatchEvents();

	CHECK(bus.PeekEvents<ev::EntityCreated>().size() == 0);
}

TEST_CASE("EventBus correctly interacts with signals", "[events]")
{
	EventBus2 bus{};
	EventTestFixture fixture{};

	fixture.signalTokens.emplace_back(
		bus.ConnectToEvent(fixture.MakeEventCallCountLambda<ev::HitCollision>()));

	auto& tk = fixture.signalTokens.front();

	CHECK(tk.IsConnected());
	
	bus.PushEvent(ev::HitCollision{});
	CHECK(bus.PeekEvents<ev::HitCollision>().size() == 1);

	bus.DispatchEvents();

	CHECK(fixture.eventCallCounts[ev::HitCollision::eventType] == 1);

	// manually disconnect, should not get called
	tk.Disconnect();

	bus.PushEvent(ev::HitCollision{});
	bus.DispatchEvents();

	CHECK(fixture.eventCallCounts[ev::HitCollision::eventType] == 1);

	// test signal token disconnect on destruction
	fixture.signalTokens.clear();
	{
	auto freeTk = bus.ConnectToEvent(fixture.MakeEventCallCountLambda<ev::TimerFired>());

	CHECK(freeTk.IsConnected());

	bus.PushEvent(events::TimerFired{});
	bus.DispatchEvents();

	CHECK(fixture.eventCallCounts[ev::TimerFired::eventType] == 1);
	}

	bus.PushEvent(events::TimerFired{});
	bus.DispatchEvents();

	CHECK(fixture.eventCallCounts[ev::TimerFired::eventType] == 1);
}

TEST_CASE("EventBus correctly dispatches events to signals", "[events]")
{
	EventBus2 bus{};
	EventTestFixture fixture{};

	fixture.signalTokens.emplace_back(
		bus.ConnectToEvent(fixture.MakeEventCallCountLambda<ev::GameControllerConnected>()));
	fixture.signalTokens.emplace_back(
		bus.ConnectToEvent(fixture.MakeEventCallCountLambda<ev::GameControllerDisconnected>()));
	fixture.signalTokens.emplace_back(
		bus.ConnectToEvent(fixture.MakeEventCallCountLambda<ev::GameControllerConnected>()));
	fixture.signalTokens.emplace_back(
		bus.ConnectToEvent(fixture.MakeEventCallCountLambda<ev::ContactCollisionBegin>()));
	fixture.signalTokens.emplace_back(
		bus.ConnectToEvent(fixture.MakeEventCallCountLambda<ev::ContactCollisionBegin>()));
	fixture.signalTokens.emplace_back(
		bus.ConnectToEvent(fixture.MakeEventCallCountLambda<ev::ContactCollisionBegin>()));

	// make sure connections successful
	for (const auto& tk : fixture.signalTokens)
	{
		CHECK(tk.IsConnected());
	}

	bus.PushEvent(ev::GameControllerDisconnected{});
	bus.PushEvent(ev::GameControllerConnected{});
	bus.PushEvent(ev::ContactCollisionBegin{});
	bus.DispatchEvents();

	CHECK(fixture.eventCallCounts[ev::GameControllerDisconnected::eventType] == 1);
	CHECK(fixture.eventCallCounts[ev::GameControllerConnected::eventType] == 2);
	CHECK(fixture.eventCallCounts[ev::ContactCollisionBegin::eventType] == 3);
}

TEST_CASE("EventBus allows connections to different input sources", "[events]")
{
	EventBus2 bus{};
	EventTestFixture fixture{};

	fixture.signalTokens.emplace_back(bus.ConnectToInput(
		GameControllerInputSource::A,
		fixture.MakeEventCallCountLambda<ev::GameControllerInput>())
	);
	fixture.signalTokens.emplace_back(bus.ConnectToInput(
		GameControllerInputSource::RightTrigger,
		fixture.MakeEventCallCountLambda<ev::GameControllerInput>())
	);
	fixture.signalTokens.emplace_back(bus.ConnectToInput(
		MouseInputSource::LeftButton,
		fixture.MakeEventCallCountLambda<ev::MouseInput>())
	);

	for (const auto& tk : fixture.signalTokens)
	{
		CHECK(tk.IsConnected());
	}

	// should get triggereed
	bus.PushEvent(ev::GameControllerInput{
		.input = { .source = GameControllerInputSource::A }
	});
	bus.PushEvent(ev::GameControllerInput{
	.input = {.source = GameControllerInputSource::RightTrigger }
	});
	bus.PushEvent(ev::MouseInput{
		.input = {.source = MouseInputSource::LeftButton }
	});

	// should not 
	bus.PushEvent(ev::GameControllerInput{
		.input = {.source = GameControllerInputSource::B }
	});
	bus.PushEvent(ev::MouseInput{
		.input = {.source = MouseInputSource::Wheel }
	});

	bus.DispatchEvents();

	CHECK(fixture.eventCallCounts[ev::GameControllerInput::eventType] == 2);
	CHECK(fixture.eventCallCounts[ev::MouseInput::eventType] == 1);
}