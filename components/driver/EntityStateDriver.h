#pragma once
#include "BaseDriver.h"
#include "../../state/EntityStateBuilder.h"

class EntityStateDriver : public BaseDriver<EntityStateDriver, EntityStates>
{
public:
	friend class Super;

	Result<EntityStateBuilder> GetStateBuilder(std::string_view stateName);

	bool EraseState(std::string_view stateName);

	Result<Void> ChangeState(std::string_view nextStateName);

	bool LinkedToCurrentState(std::string_view stateLink) const;

private:
	explicit EntityStateDriver(Entity ent) : BaseDriver(ent) {}

	static EntityState* GetCurrentEntityState(EntityStates& states);
};