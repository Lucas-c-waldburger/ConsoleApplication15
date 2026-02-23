#include "UserSystemScheduler.h"

void UserSystemScheduler::UpdateSystems(Phase phase, float dt)
{
	for (const size_t idx : phaseIndices_[phase])
	{
		assert(idx < updateSlots_.size());

		auto& slot = updateSlots_[idx];
		assert(slot.updateFn);
		assert(slot.instance);

		std::invoke(slot.updateFn, slot.instance, dt);
	}
}
