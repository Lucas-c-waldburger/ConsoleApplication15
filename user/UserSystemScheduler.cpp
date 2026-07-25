#include "UserSystemScheduler.h"

void UserSystemScheduler::Reset()
{
	for (size_t i = enum_start_v<Phase>; i < enum_size_v<Phase>; ++i)
	{
		phaseToSlotKeys_[static_cast<Phase>(i)].clear();
	}

	systems_.clear();
	slots_.clear();
}

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