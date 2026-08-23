#pragma once
#include "System.h"
#include "../scripting/ScriptTable.h"
#include "../core/Algorithms.h"
#include "../core/SizedEnumMap.h"
#include "Phase.h"
#include "../user/UserSystemScheduler.h"
#include <unordered_set>
#include <deque>

class ScriptableUserSubsystem : public System
{
public:
	struct Instance
	{
		ScriptTable::TableId id = std::numeric_limits<ScriptTable::TableId>::max();
		sol::function update;
	};

	template <Phase ph>
	void Update(float dt)
	{
		for (auto& instance : phaseToInstance_[ph])
		{
			sol::protected_function_result result = instance.update(dt);
			if (!result.valid())
			{
				sol::error err = result;
				LOG_ERROR("Error calling system update script : {}", err.what());
			}
		}
	}

	Result<Void> AddSystemScript(const ScriptTable& table, Phase phase);
	bool RemoveSystemScript(ScriptTable::TableId id);
	void Clear();

private:
	std::unordered_map<ScriptTable::TableId, Phase> tableIdToPhase_;
	SizedEnumMap<Phase, std::vector<Instance>> phaseToInstance_;
};