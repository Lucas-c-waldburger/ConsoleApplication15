#include "GuiEditEvents.h"
#include "../../../ecs/Ecs.h"
#include "../InspectorCommon.h"
#include "GuiEditPropertyTable.h"

#if IMGUI_ENABLED

namespace ui {

PropertyEditState GuiEditProperty(CollisionEventParticipantContext& ctx)
{
	if (ImGui::BeginCombo(ctx.invisibleLabel.data(), ctx.currentName.c_str()))
	{
		for (const auto& e : ctx.allEntities)
		{
			assert(e.HasComponent<Name>());
			const auto& name = e.GetComponent<Name>().value;

			if (name == ctx.nameToTest)
			{
				continue;
			}

			const bool selected = (name == ctx.currentName);
			if (ImGui::Selectable(name.c_str(), &selected))
			{
				ctx.eventParticipantId = e.GetID();
				ctx.participantEntity = e;
				ctx.currentName = name;
			}
		}

		ImGui::EndCombo();
	}

	return EvaluatePropertyState();
}

PropertyEditState GuiEditProperty(CollisionEventShapesContext& ctx)
{
	const auto shapeInfo = GetCollisionDataShapeInfo(ctx.participantEntity, ctx.collisionData);

	if (ImGui::BeginCombo(ctx.invisibleLabel.data(), shapeInfo.current.label.c_str()))
	{
		if (!IsValidCollisionParticipant(ctx.participantEntity))
		{
			ctx.eventParticipantId = kInvalidEntity;
		}
		else
		{
			for (const auto& infoElem : shapeInfo.all)
			{
				bool selected = (infoElem.handle == shapeInfo.current.handle);

				if (ImGui::Selectable(infoElem.label.c_str(), &selected))
				{
					ctx.collisionData.entity = infoElem.entityId;
					ctx.collisionData.shapeHandle = infoElem.handle;
				}
			}
		}

		ImGui::EndCombo();
	}

	return EvaluatePropertyState();
}

PropertyEditState GuiDrawProperty(const CallbackInfoEntryContext& ctx)
{
	assert(ctx.i < ctx.cbInfo.eventNames.size());
	assert(ctx.i < ctx.cbInfo.scriptFileNames.size());
	assert(ctx.i < ctx.cbInfo.tableFunctionNames.size());

	//Property("eventType", ctx.cbInfo.eventNames[ctx.i]);
	Property("table", ctx.cbInfo.scriptFileNames[ctx.i]);
	Property("function", ctx.cbInfo.tableFunctionNames[ctx.i]);

	return PropertyEditState::None;
}


PropertyEditState GuiEditProperty(events::ContactCollisionBegin&) { return PropertyEditState::None; }
PropertyEditState GuiEditProperty(events::ContactCollisionEnd&) { return PropertyEditState::None; }
PropertyEditState GuiEditProperty(events::SensorCollisionBegin&) { return PropertyEditState::None; }
PropertyEditState GuiEditProperty(events::SensorCollisionEnd&) { return PropertyEditState::None; }
PropertyEditState GuiEditProperty(events::HitCollision&) { return PropertyEditState::None; }



} // ui

#endif