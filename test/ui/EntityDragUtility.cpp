#include "EntityDragUtility.h"

#if IMGUI_ENABLED
#include "../../ecs/Ecs.h"
#include "../../camera/Camera.h"
#include "GuiMouse.h"
#include "InspectorCommon.h"
#include "ComponentEditHistory.h"

namespace ui {

void EntityDragUtility::SetOverrideDelta(const Entity& e, SDL_FPoint mouseAbsPos)
{
	const auto& rb = e.GetComponent<RigidBody>();

	overrideDelta_ = mouseAbsPos - rb.body.GetData().GetPosition();
}

void EntityDragUtility::AdjustEntityPosition(Entity& e)
{
	if (e.HasComponent<RigidBody>(HasValidBody))
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

	if (!e.HasComponent<Transform>())
	{
		isDragging_ = false;
		return;
	}

	if (GuiMouse::InsideEditorWindow())
	{
		if (isDragging_)
		{
			ComponentEditHistory::EndComponentEdit(e, e.GetComponent<Transform>());
			LOG_DEBUG_FMT("Ended Component Edit : Transform (Size: {})",
				ComponentEditHistory::GetRecordsSize());
		}

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
				ComponentEditHistory::BeginComponentEdit(e, e.GetComponent<Transform>());
				LOG_DEBUG_FMT("Began Component Edit : Transform (Size: {})",
					ComponentEditHistory::GetRecordsSize());

				isDragging_ = true;

				if (e.HasComponent<RigidBody>(HasValidBody))
				{
					SetOverrideDelta(e, mousePos);
				}
			}
		}
		else
		{
			ComponentEditHistory::EndComponentEdit(e, e.GetComponent<Transform>());
			LOG_DEBUG_FMT("Ended Component Edit : Transform (Size: {})",
				ComponentEditHistory::GetRecordsSize());

			isDragging_ = false;
		}
	}
	else if (GuiMouse::IsLeftReleased() || !GuiMouse::IsLeftHeld())
	{
		if (isDragging_)
		{
			ComponentEditHistory::EndComponentEdit(e, e.GetComponent<Transform>());
			LOG_DEBUG_FMT("Ended Component Edit : Transform (Size: {})",
				ComponentEditHistory::GetRecordsSize());
		}

		isDragging_ = false;
	}

	if (isDragging_)
	{
		AdjustEntityPosition(e);
	}
}

} // ui

#endif