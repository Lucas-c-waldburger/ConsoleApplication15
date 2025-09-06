#pragma once
#include <vector>
#include "../core/Result.h"
#include "../core/commonObjects.h"
#include "System.h"
#include "../state/EntityState.h"


class EntityStateSystem : public System
{
public:
	void Update(float delta);

	template <SomeEntityState T> 
	Result<Void> RegisterState()
	{
		const size_t id = EntityState<T>::GetStateID();
		/*if (id >= stateTables_.size())
		{
			stateTables_.resize(id + 1);
		}*/

		if (stateTables_.contains(id))
		{
			return MAKE_ERROR_FMT("EntityState with ID '{}'"
				" already registered with Entity State System", id);
		}

		stateTables_[id] = EntityState<T>::GetStateTable();

		return Void{};
	}

private:
	//bool StateTableRegistered(size_t id) const
	//{
	//	return id < stateTables_.size() && (stateTables_[id].onEnter || 
	//										stateTables_[id].onExit || 
	//										stateTables_[id].onUpdate);
	//}

	//std::vector<EntityStateTable> stateTables_;
	std::unordered_map<size_t, EntityStateTable> stateTables_;
};