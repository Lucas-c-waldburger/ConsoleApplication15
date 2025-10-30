//#pragma once
//#include <cassert>
//#include "Fixtures.h"
//
//
//namespace ui {
//
//class Button
//{
//public:
//	enum Color { Red = 0, Blue, Yellow, Green };
//	static constexpr std::array kButtonColors = {
//		"button_red",
//		"button_blue",
//		"button_yellow",
//		"button_green"
//	};
//
//	struct ButtonSprites
//	{
//		SpriteRenderable up;
//		SpriteRenderable down;
//		SpriteRenderable hover;
//	};
//
//	struct ButtonCallbacks
//	{
//		fu2::unique_function<void()> onClick;
//		fu2::unique_function<void()> onHover;
//	};
//
//	struct ButtonState
//	{
//		enum : uint8_t
//		{
//			Hovering = 1 << 0,
//			ClickedInside = 1 << 1
//		};
//		uint8_t value = 0;
//	};
//
//	struct Params
//	{
//		Dimensions<int> dimensions;
//		SDL_FPoint position = { 0.0f, 0.0f };
//		ButtonSprites sprites;
//		std::optional<TextRenderable> text;
//		ButtonCallbacks callbacks;
//	};
//
//	Button(EventBus2& bus, Params&& params);
//
//	void SetPosition(SDL_FPoint pos);
//	void SetDimensions(Dimensions<int> dim);
//
//	//template <typename Fn> requires std::convertible_to<Fn, fu2::function<void()>>
//	//void SetOnClick(Fn&& fn) { callbacks_.onClick = std::forward<Fn>(fn); }
//	//template <typename Fn> requires std::convertible_to<Fn, fu2::function<void()>>
//	//void SetOnHover(Fn&& fn) { callbacks_.onHover = std::forward<Fn>(fn); }
//
//private:
//	auto MakeOnClickCallback();
//	//auto MakeOnHoverCallback();
//
//	Entity self_;
//	ButtonSprites sprites_;
//	SDL_Rect boundingBox_ = { 0, 0, 0, 0 };
//	ButtonState state_;
//	ButtonCallbacks callbacks_;
//};
//
//class Workspace
//{
//public:
//	 
//	Result<Button> PlaceButton(SDL_FPoint position, Dimensions<int> dimensions,
//							   Button::Color color);
//
//private:
//	TextureRepository* textureRepo_ = nullptr;
//	EventBus2* eventBus_ = nullptr;
//};
//
//
//
//
//
//
//}
//
//
//
//
//
//
//namespace test {
//
//
//bool ProceedProcessInput(const Entity& entity, const events::GameControllerInput& ev)
//{
//	assert(entity.IsValid());
//
//	if (!(entity.HasComponent<GameControllerState>() &&
//		  entity.HasComponent<ActiveAudio>()))
//	{
//		return false;
//	}
//
//	auto& gc = entity.GetComponent<GameControllerState>();
//	if (gc.joystickID != ev.joystickID)
//	{
//		return false;
//	} 
//
//	const auto& aa = entity.GetComponent<ActiveAudio>();
//	if (aa.status == AudioStatus::Staged || aa.status == AudioStatus::Stopped)
//	{
//		return false;
//	}
//
//	return true;
//}
//
//auto MakeConnectControllerCallback(Entity& entity)
//{
//	return [entity](const events::GameControllerConnected& ev) mutable {
//		if (!entity.HasComponent<GameControllerState>()) { return; }
//
//		auto& gc = entity.GetComponent<GameControllerState>();
//		if (gc.joystickID == GameController::kInvalidJoystickID)
//		{
//			gc.joystickID = ev.joystickID;
//		}
//	};	
//}
//
//auto MakeDisconnectControllerCallback(Entity& entity)
//{
//	return [entity](const events::GameControllerDisconnected& ev) mutable {
//		if (!entity.HasComponent<GameControllerState>()) { return; }
//
//		auto& gc = entity.GetComponent<GameControllerState>();
//		if (gc.joystickID == ev.joystickID)
//		{
//			gc.joystickID = GameController::kInvalidJoystickID;
//		}
//	};
//}
//
//auto MakePlayPauseCommandCallback(Entity& entity)
//{
//	return [entity](const events::GameControllerInput& ev) mutable {
//		if (!ProceedProcessInput(entity, ev)) { return; }
//
//		const auto& activeAudio = entity.GetComponent<ActiveAudio>();
//		auto& updateReq = entity.AddComponent<AudioUpdateRequest>();
//		updateReq.instanceId = activeAudio.instanceId;
//		updateReq.command = (activeAudio.status == AudioStatus::Playing)
//			? AudioPlayCommand::Pause : AudioPlayCommand::Resume;
//	};
//}
//
//auto MakeVolumeChangeCallback(Entity& entity, int amount)
//{
//	return [entity, amount](const events::GameControllerInput& ev) mutable {
//		if (!ProceedProcessInput(entity, ev)) { return; }
//
//		const auto& activeAudio = entity.GetComponent<ActiveAudio>();
//		auto& updateReq = entity.AddComponent<AudioUpdateRequest>();
//		updateReq.instanceId = activeAudio.instanceId;
//		updateReq.settings.volume = activeAudio.settings.volume + amount;
//	};
//}
//
//void MapAudioPlayControls(Entity& entity, EventBus2& bus)
//{
//	using namespace events;
//	using Src = GameControllerInputSource;
//
//	auto& tks = entity.AddComponent<SignalTokenStorage>().signalTokens;
//
//	tks.emplace_back(bus.ConnectToInput(Src::A, 
//		MakePlayPauseCommandCallback(entity)));
//	tks.emplace_back(bus.ConnectToInput(Src::DPadUp, 
//		MakeVolumeChangeCallback(entity, 10)));
//	tks.emplace_back(bus.ConnectToInput(Src::DPadDown,
//		MakeVolumeChangeCallback(entity, -10)));
//}
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//}