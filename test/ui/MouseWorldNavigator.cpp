#include "MouseWorldNavigator.h"

#if IMGUI_ENABLED

#include "../../sdl/SDLite.h"
#include "../../events/EventBus2.h"

namespace {

template <typename...Args> requires (std::same_as<Args, MouseInputSource> && ...)
constexpr bool AnyPressed(const MouseState& mouseState, Args...args)
{
	return ((mouseState.inputs[args].state == InputState::Pressed) || ...);
}

} // unnamed

Result<Void> MouseWorldNavigator::Init(const MouseEventHandler& mouseEvHandler, 
									   Camera& cam)
{
	using CTX = MouseWorldNavigatorContext;

	if (CTX::mouseEventHandler)
	{
		LOG_WARNING("MouseWorldNavigatorContext's mouse event handler "
			"was already assigned");
		return Void{};
	}

	CTX::mouseEventHandler = &mouseEvHandler;

	if (CTX::navCursor)
	{
		LOG_WARNING("MouseWorldNavigatorContext's nav cursor was already allocated");
		return Void{};
	}

	CTX::navCursor = MakeUniqueCursor(SDL_SYSTEM_CURSOR_SIZEALL);
	assert(CTX::navCursor);

	CTX::camera = &cam;

	return Void{};
}

void MouseWorldNavigator::Update(double deltaTime)
{
	using CTX = MouseWorldNavigatorContext;
	assert(CTX::navCursor);
	assert(CTX::mouseEventHandler);
	assert(CTX::camera);

	using Source = MouseInputSource;

	auto mouseState = CTX::mouseEventHandler->GetMouseState();

	const auto& middleBtn = mouseState.inputs[Source::MiddleButton];
	const auto& leftBtn = mouseState.inputs[Source::LeftButton];
	const auto& rightBtn = mouseState.inputs[Source::RightButton];

	const bool inFreeScrollMode = CTX::scrollOffset.has_value();

	if (inFreeScrollMode)
	{
		const bool shouldExitMode = AnyPressed(mouseState, Source::MiddleButton, 
														   Source::LeftButton, 
														   Source::RightButton);
		if (shouldExitMode)
		{
			SDL_SetCursor(SDL_GetDefaultCursor());
			CTX::scrollOffset.reset();
		}
		else
		{
			HandleFreeScrollMode(mouseState, deltaTime);
		}

		return;
	}
	else if (middleBtn.state == InputState::Pressed)
	{
		SDL_SetCursor(CTX::navCursor.get());
		CTX::scrollOffset.emplace(SDL_FPoint{ 0.0f, 0.0f });

		return;
	}

	//bool pressed = leftBtn.state == InputState::Pressed;
	//if (leftBtn.state == InputState::Pressed)
	//{
	//	CTX::leftButtonPressed = !CTX::leftButtonPressed;
	//}
	//else if (leftBtn.state == InputState::Held)
	//if (leftBtn.state == InputState::Released)
	//{

	//}


	//if (middleBtn.state == InputState::Pressed && !inNavMode)
	//{
	//	SDL_SetCursor((!CTX::inNavMode 
	//		? CTX::navCursor.get() 
	//		: SDL_GetDefaultCursor()));

	//	auto cursorPos = mouseState.cursorValue.position.absolute;
	//	CTX::scrollOrigin.emplace(cursorPos);

	//	

	//	CTX::inNavMode = !CTX::inNavMode;

	//	if (CTX::inNavMode)
	//	{
	//		auto relPos = mouseState.cursorValue.position.relative;


			//auto targets = ECS::GetAllEntitiesWith<CameraTarget>();
			//for (auto& e : targets)
			//{
			//	e.RemoveComponent<CameraTarget>();
			//}

			//CTX::mouseEntity.AddComponent(CameraTarget{ .followSpeed = 50 });
		//}
		//else
		//{
			//CTX::mouseEntity.RemoveComponent<CameraTarget>();
		//}
	//}


	//if (CTX::inNavMode)
	//{
	//	CTX::mouseEntity.GetComponent<Transform>().position 
	//		= mouseState.cursorValue.position.absolute;
	//}

	/*auto mousePos = mouseState.cursorValue.position.absolute;
	auto [posX, posY] = Dimensions<int>{
		static_cast<int>(mousePos.x),
		static_cast<int>(mousePos.y)
	};

	auto vp = CTX::camera->GetViewport();
	auto [winW, winH] = SDLite::Window().GetSize();

	SDL_Point scrollAmount = { 0, 0 };

	if (posX > 0 && posX < kScreenEdgeScrollBuffer.w)
	{
		scrollAmount.x -= kFrameScreenScrollAmount;
	}
	else if (posX > winW - kScreenEdgeScrollBuffer.w && posX < winW)
	{
		scrollAmount.x += kFrameScreenScrollAmount;
	}

	if (posY > 0 && posY < kScreenEdgeScrollBuffer.h)
	{
		scrollAmount.y -= kFrameScreenScrollAmount;
	}
	else if (posY > winH - kScreenEdgeScrollBuffer.h && posY < winH)
	{
		scrollAmount.y += kFrameScreenScrollAmount;
	}

	if (scrollAmount.x != 0 || scrollAmount.y != 0)
	{
		CTX::camera->Pan({ static_cast<float>(scrollAmount.x),
						   static_cast<float>(scrollAmount.y) });
	}*/
}

void MouseWorldNavigator::HandleFreeScrollMode(const MouseState& mouseState, 
											   double deltaTime)
{
	using CTX = MouseWorldNavigatorContext;

	*CTX::scrollOffset += mouseState.values.cursor.relativePos;

	auto dir = *CTX::scrollOffset;
	float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);
	if (dist > 0.0f)
	{
		dir.x /= dist;
		dir.y /= dist;
	}

	float t = std::min(dist / kScrollMaxDistance, 1.0f);
	float speed = t * kScrollMaxSpeed;

	CTX::camera->Pan({
		dir.x * speed * static_cast<float>(deltaTime),
		dir.y * speed * static_cast<float>(deltaTime)
	});
}

#endif