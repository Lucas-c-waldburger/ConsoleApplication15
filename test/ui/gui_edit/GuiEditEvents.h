#pragma once
#include "../../../FeatureFlags.h"

#if IMGUI_ENABLED
#include <imgui.h>
#include "../PropertyEditState.h"
#include "../../../events/data/EventDataIncludes.h"
#include "../../../ecs/EntityT.h"

class Entity;

namespace ui {

struct CollisionDataShapeInfo;

struct CollisionEventParticipantContext
{
	std::string_view invisibleLabel;
	std::vector<Entity>& allEntities;
	std::string& currentName;
	std::string_view nameToTest;
	Entity& participantEntity;
	Entity_t& eventParticipantId;
};

struct CollisionEventShapesContext
{
	std::string_view invisibleLabel;
	CollisionData& collisionData;
	Entity& participantEntity;
	Entity_t& eventParticipantId;
};

PropertyEditState GuiEditProperty(CollisionEventParticipantContext& ctx);
PropertyEditState GuiEditProperty(CollisionEventShapesContext& ctx);

PropertyEditState GuiEditProperty(events::ContactCollisionBegin&);
PropertyEditState GuiEditProperty(events::ContactCollisionEnd&);
PropertyEditState GuiEditProperty(events::SensorCollisionBegin&);
PropertyEditState GuiEditProperty(events::SensorCollisionEnd&);
PropertyEditState GuiEditProperty(events::HitCollision&);

} // ui

#endif