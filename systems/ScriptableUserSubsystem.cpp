#include "ScriptableUserSubsystem.h"

Result<Void> ScriptableUserSubsystem::AddSystemScript(const ScriptTable& table, Phase phase)
{
	if (!table.IsValid())
	{
		return MAKE_ERROR("ScriptTable was invalid");
	}

	if (tableIdToPhase_.contains(table.GetTableId()))
	{
		return MAKE_ERROR("ScriptTable is already tied to a script instance");
	}

	const auto& fnSigs = table.GetFunctionSignatures();

	if (auto it = fnSigs.find("update"); it != fnSigs.end())
	{
		const bool validArgs = it->second.empty() || (it->second.size() == 1 &&
							   it->second.front() == GetNativeLuaTypeId("number"));
		if (!validArgs)
		{
			return MAKE_ERROR("unsupported arguments for update function. Update function must "
				"either take no arguments, or a single 'number' argument");
		}
	}
	else
	{
		return MAKE_ERROR("ScriptTable does not contain function named 'update'");
	}

	if (auto it = fnSigs.find("init"); it != fnSigs.end())
	{
		if (!it->second.empty())
		{
			return MAKE_ERROR("unsupported arguments for init function. init function must "
				"take no arguments");
		}

		sol::protected_function_result initResult = table.Data()["init"]();
		if (!initResult.valid())
		{
			sol::error err = initResult;

			return MAKE_ERROR_FMT("Error calling system init script : {}", err.what());
		}
	}

	tableIdToPhase_.try_emplace(table.GetTableId(), phase);
	phaseToInstance_[phase].emplace_back(table.GetTableId(), table.Data()["update"]);

	return kVoid;
}

bool ScriptableUserSubsystem::RemoveSystemScript(ScriptTable::TableId id)
{
	auto it = tableIdToPhase_.find(id);
	if (it == tableIdToPhase_.end())
	{
		return false;
	}

	core::EraseIf(phaseToInstance_[it->second], [id](const auto& instance) {
		return instance.id == id;
		});

	tableIdToPhase_.erase(id);

	return true;
}

void ScriptableUserSubsystem::Clear()
{
	tableIdToPhase_.clear();

	for (size_t i = enum_start_v<Phase>; i < enum_size_v<Phase>; ++i)
	{
		phaseToInstance_[static_cast<Phase>(i)].clear();
	}
}