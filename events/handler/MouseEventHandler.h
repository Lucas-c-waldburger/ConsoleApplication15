#pragma once
#include "../../components/MouseStateComponent.h"
#include "../../inputs/mouse/MouseInputUpdater.h"
#include <bitset>

class EventBus2;

class MouseEventHandler
{
public:
	MouseEventHandler() = default;
	~MouseEventHandler() = default;

	void HandleMouseEvent(const SDL_Event& ev);
	void Finalize(float delta, EventBus2& bus);

	MouseState GetMouseState() const;

private:
	void UpdateMouseStateComponents();

	MouseInputUpdater inputUpdater_;
};