#pragma once
#include "../../components/MouseStateComponent.h"
#include "../../inputs/mouse/MouseInputUpdater.h"
#include <bitset>

class EventBus;

class MouseEventHandler
{
public:
	MouseEventHandler() = default;
	~MouseEventHandler() = default;

	void HandleMouseEvent(const SDL_Event& ev);
	void Finalize(float delta, EventBus& bus);

	MouseState GetMouseState() const;

private:
	void UpdateMouseStateComponents();

	MouseInputUpdater inputUpdater_;
};