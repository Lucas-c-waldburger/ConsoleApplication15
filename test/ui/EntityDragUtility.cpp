#include "EntityDragUtility.h"

#if IMGUI_ENABLED
#include "../../ecs/Ecs.h"
#include "../../camera/Camera.h"
#include "GuiMouse.h"
#include "InspectorCommon.h"

namespace ui {

namespace {

bool HasValidBody(const Entity& e)
{
	return e.HasComponent<RigidBody>([](const auto& rb) {
		return rb.body.GetData().IsValid();
	});
}

} // unnamed

void EntityDragUtility::SetOverrideDelta(const Entity& e, SDL_FPoint mouseAbsPos)
{
	const auto& rb = e.GetComponent<RigidBody>();

	overrideDelta_ = mouseAbsPos - rb.body.GetData().GetPosition();
}

void EntityDragUtility::AdjustEntityPosition(Entity& e)
{
	if (HasValidBody(e))
	{
		auto& body = WriteAccessor<B2Body>{}(e.GetComponent<RigidBody>().body);

		body.SetPosition(GuiMouse::GetPosition() - overrideDelta_);
	}
	else if (e.HasComponent<Transform>())
	{
		e.GetComponent<Transform>().position += GuiMouse::GetRelativePosition();
	}
}

void EntityDragUtility::Reset()
{
	isDragging_ = false;
	overrideDelta_ = { 0.0f, 0.0f };
}

void EntityDragUtility::Update(Entity& e, const Camera& cam)
{
	auto mousePos = GuiMouse::GetPosition();

	if (GuiMouse::InsideEditorWindow())
	{
		isDragging_ = false;
		return;
	}

	if (!e.HasComponent<Transform>())
	{
		isDragging_ = false;
		return;
	}

	if (GuiMouse::IsLeftClicked())
	{
		if (!isDragging_)
		{
			auto r = GetScreenRectForEntity(e, cam);

			if (PointInsideRect(r, mousePos))
			{
				isDragging_ = true;

				if (HasValidBody(e))
				{
					SetOverrideDelta(e, mousePos);
				}
			}
		}
		else
		{
			isDragging_ = false;
		}
	}
	else if (GuiMouse::IsLeftReleased() || !GuiMouse::IsLeftHeld())
	{
		isDragging_ = false;
	}

	if (!isDragging_)
	{
		return;
	}

	AdjustEntityPosition(e);
}

} // ui

#endif