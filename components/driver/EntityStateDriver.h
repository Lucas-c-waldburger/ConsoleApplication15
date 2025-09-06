#pragma once
#include "BaseDriver.h"
#include "../../callbacks/StateTransitionCallbackRegistry.h"

//class EntityStateDriver : public BaseDriver<EntityStateDriver, EntityStates>
//{
//public:
//	friend class Super;
//
//	Result<Void> AddState(EntityState&& newState);
//
//	bool RemoveState(std::string_view stateName, bool removeLinks = false);
//
//	Result<Void> ChangeState(std::string_view nextStateName);
//
//	bool HasState(std::string_view stateName) const;
//
//	HashName GetCurrentStateName() const;
//
//	bool LinkedToCurrentState(std::string_view stateLink) const;
//
//private:
//	explicit EntityStateDriver(Entity ent) : BaseDriver(ent) {}
//
//	static EntityState* GetCurrentEntityState(EntityStates& states);
//};