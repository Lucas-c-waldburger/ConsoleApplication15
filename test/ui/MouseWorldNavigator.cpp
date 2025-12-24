#include "MouseWorldNavigator.h"
#include "../../sdl/SDLite.h"
#include "../../events/EventBus2.h"

Result<Void> MouseWorldNavigator::Init(const MouseEventHandler& mouseEvHandler)
{
	using CTX = MouseWorldNavigatorContext;

	if (CTX::mouseEntity.IsValid())
	{
		LOG_WARNING("MouseWorldNavigatorContext's mouse entity was already created");
		return Void{};
	}

	CTX::mouseEntity = ECS::CreateEntity();
	assert(CTX::mouseEntity.IsValid());

	CTX::mouseEntity.AddComponent<Transform>();

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

	return Void{};
}

void MouseWorldNavigator::Update()
{
	using CTX = MouseWorldNavigatorContext;
	assert(CTX::mouseEntity.IsValid());
	assert(CTX::mouseEntity.HasComponent<Transform>());
	assert(CTX::navCursor);
	assert(CTX::mouseEventHandler);

	auto mouseState = CTX::mouseEventHandler->GetMouseState();
	const auto& middleBtnState = mouseState.inputs[MouseInputSource::MiddleButton];

	if (middleBtnState.state == InputState::Pressed)
	{
		SDL_SetCursor((!CTX::inNavMode 
			? CTX::navCursor.get() 
			: SDL_GetDefaultCursor()));

		CTX::inNavMode = !CTX::inNavMode;

		if (CTX::inNavMode)
		{
			auto targets = ECS::GetAllEntitiesWith<CameraTarget>();
			for (auto& e : targets)
			{
				e.RemoveComponent<CameraTarget>();
			}

			CTX::mouseEntity.AddComponent(CameraTarget{ .followSpeed = 50 });
		}
		else
		{
			CTX::mouseEntity.RemoveComponent<CameraTarget>();
		}
	}

	if (CTX::inNavMode)
	{
		CTX::mouseEntity.GetComponent<Transform>().position 
			= mouseState.cursorValue.position.absolute;
	}

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