#pragma once
#include "../../components/MouseStateComponent.h"
#include <bitset>

class MouseEventHandler
{
public:
	void HandleMotionEvent(const SDL_Event& ev);
	void HandleButtonEvent(const SDL_Event& ev);
	void HandleWheelEvent(const SDL_Event& ev);

	void UpdateEntities();

private:
	struct InputCache
	{
		static constexpr size_t kMousePositionShiftedIndex = 6;

		MouseState data;
		std::bitset<6> updatedTracker;
		void MarkUpdated(size_t shiftedIdx);
		void UpdateSkippedInputs();
	};

	InputCache inputCache;
};