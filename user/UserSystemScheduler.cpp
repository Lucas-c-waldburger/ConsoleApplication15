#include "UserSystemScheduler.h"

//void UserSystemScheduler::UpdateSystems(Phase phase, float dt)
//{
//	for (const size_t idx : phaseIndices_[phase])
//	{
//		assert(idx < updateSlots_.size());
//
//		auto& slot = updateSlots_[idx];
//		assert(slot.updateFn);
//		assert(slot.instance);
//
//		std::invoke(slot.updateFn, slot.instance, dt);
//	}
//}

void UserSystemScheduler::UpdateSystems(Phase phase, float dt)
{
	for (const auto& key : phaseToSlotKeys_[phase])
	{
		const auto sysId = static_cast<size_t>(key.systemId);
		assert(sysId < slots_.size());

		auto& slot = slots_[sysId];
		assert(key.operationIndex < slot.operationList.size());

		auto& op = slot.operationList[key.operationIndex];
		assert(op.phase == phase);
		assert(op.fn != nullptr);

		std::invoke(op.fn, slot.instance, dt);
	}
}