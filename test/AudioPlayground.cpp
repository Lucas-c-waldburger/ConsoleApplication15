#include "AudioPlayground.h"

//test::AudioPlayground::AudioPlayground(EventBus2& bus) : self_(ECS::CreateEntity())
//{
//	using enum GameControllerInputSource;
//
//	self_.AddComponent<GameControllerState>();
//	
//	auto& tks = self_.AddComponent<SignalTokenStorage>().signalTokens;
//
//	tks.emplace_back(bus.ConnectToEvent(MakeConnectControllerCallback(self_)));
//	tks.emplace_back(bus.ConnectToEvent(MakeDisconnectControllerCallback(self_)));
//
//	tks.emplace_back(bus.ConnectToInput(A, MakePlayPauseCommandCallback(self_)));
//	tks.emplace_back(bus.ConnectToInput(DPadUp, MakeVolumeChangeCallback(self_, 10)));
//	tks.emplace_back(bus.ConnectToInput(DPadDown, MakeVolumeChangeCallback(self_, -10)));
//}
