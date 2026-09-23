#pragma once
#include <bitset>
#include "MouseInputMap.h"
#include "../../components/MouseStateComponent.h"

class EventBus;
struct RenderTargetState;

class MouseInputUpdater
{
public:
	MouseInputUpdater() : inputs_(MakeInputMap<MouseInputMap>()) {}
	~MouseInputUpdater() = default;

	void Update(const SDL_Event& ev, const RenderTargetState& renderTargetState);
	void FinalizeAndPushEvents(float dt, EventBus& bus);

	MouseInputMap& GetInputMap() { return inputs_; }
	const MouseInputMap& GetInputMap() const { return inputs_; }

	MouseState ToMouseStateComponent() const;

private:
	class Tracker
	{
	public:
		constexpr Tracker() : updated() { timestamps.fill(0); }

		std::bitset<enum_size_v<MouseInputSource>> updated;
		std::array<uint32_t, enum_size_v<MouseInputSource>> timestamps;
	};

	MouseInputMap inputs_;
	MouseInputValues values_;
	Tracker tracker_;
};