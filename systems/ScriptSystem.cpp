#include "ScriptSystem.h"
#include "../ecs/Ecs.h"
#include "../components/util/ComponentValidPreds.h"
#include <ranges>

namespace {

std::optional<Phase> TryParsePhaseFromMetatable(const sol::table& fnTable)
{
	sol::table meta = fnTable[sol::metatable_key];
	if (!meta.valid())
	{
		return std::nullopt;
	}

	Phase phase = meta.get_or("__phase", Phase::Invalid);

	return phase;
}


void InvalidateEntityScriptTables(ScriptTable::TableId tableId)
{
	auto es = ECS::GetAllEntitiesWith<Script>([tableId](const Script& script) {
		return script.table.GetTableId() == tableId;
	});

	for (auto& e : es)
	{
		e.GetComponent<Script>().table = {};
	}
}

void EraseScriptCallbacks(Entity& e)
{
	if (!e.HasComponent<SignalTokenStorage>())
	{
		return;
	}

	auto& tks = e.GetComponent<SignalTokenStorage>().signalTokens;

	core::EraseIf(tks, [](const auto& tk) {
		return tk.type == EntityCallbackToken::Type::Script; }
	);
}

} // unnamed

ScriptSystem::~ScriptSystem()
{
	auto es = ECS::GetAllEntitiesWith<Script>(&ScriptValid);
	for (auto& e : es)
	{
		e.GetComponent<Script>().table = {};
	}
}

Result<ScriptTable::TableId> ScriptSystem::AddTable(ScriptTableDescriptor&& descriptor)
{
	if (descriptor.tableType != ScriptTable::TableType::SystemTable)
	{
		descriptor.tableType = ScriptTable::TableType::FunctionTable;
	}

	TRY(tables_.AddTable(std::move(descriptor), state_), tableId);

	auto infoOp = tables_.GetTableInfo<&ScriptTableInfo::tableType, 
									   &ScriptTableInfo::systemPhase>(tableId);

	assert(infoOp.has_value());

	auto [tableType, sysPhase] = *infoOp;

	if (tableType == ScriptTable::TableType::SystemTable)
	{
		assert(sysPhase != Phase::Invalid);

		auto* table = tables_.GetTable(tableId);
		assert(table);

		auto result = scriptableUserSubsystem_.AddSystemScript(*table, sysPhase);
		if (!result.Success())
		{
			tables_.RemoveTable(tableId);

			return result.GetError();
		}
	}

	return tableId;
}

Result<Void> ScriptSystem::ReloadTable(ScriptTable::TableId tableId)
{
	if (!tables_.ContainsTable(tableId))
	{
		return MAKE_ERROR_FMT("Script table with Id '{}' not found", tableId);
	}

	auto oldInfoOp = tables_.GetTableInfo<&ScriptTableInfo::tableType, 
										  &ScriptTableInfo::systemPhase>(tableId);
	assert(oldInfoOp);

	auto oldTableType = std::get<0>(*oldInfoOp);
	auto oldSysPhase = std::get<1>(*oldInfoOp);

	TRY(tables_.ReloadTable(tableId, state_));

	auto newInfoOp = tables_.GetTableInfo<&ScriptTableInfo::tableType,
										  &ScriptTableInfo::systemPhase>(tableId);
	assert(newInfoOp);

	auto newTableType = std::get<0>(*newInfoOp);
	auto newSysPhase = std::get<1>(*newInfoOp);

	if (oldTableType == ScriptTable::TableType::SystemTable)
	{
		if (newTableType != ScriptTable::TableType::SystemTable)
		{
			scriptableUserSubsystem_.RemoveSystemScript(tableId);
		}
		else if (oldSysPhase != newSysPhase)
		{
			scriptableUserSubsystem_.RemoveSystemScript(tableId);

			auto* table = tables_.GetTable(tableId);
			assert(table);

			TRY(scriptableUserSubsystem_.AddSystemScript(*table, newSysPhase));
		}
	}

	const auto tableName = tables_.GetTableInfo<&ScriptTableInfo::name>(tableId);
	assert(tableName.has_value());

	LOG_DEBUG_FMT("Script table '{}' reloaded", *tableName);

	return kVoid;
}

ScriptTableView ScriptSystem::GetTableView(ScriptTable::TableId tableId) const
{
	return tables_.GetTableView(tableId);
}

void ScriptSystem::Reset()
{
	auto es = ECS::GetAllEntitiesWith<Script>();
	for (auto& e : es)
	{
		e.GetComponent<Script>().table = {};
	}

	tables_.Clear();
	scriptableUserSubsystem_.Clear();
	state_ = {};
}

bool ScriptSystem::RemoveTable(ScriptTable::TableId tableId)
{
	if (!tables_.ContainsTable(tableId))
	{
		return false;
	}

	const auto tableType = tables_.GetTableInfo<&ScriptTableInfo::tableType>(tableId);
	assert(tableType.has_value());

	switch (*tableType)
	{
	case ScriptTable::TableType::FunctionTable:
		InvalidateEntityScriptTables(tableId);
		break;
	case ScriptTable::TableType::SystemTable:
		scriptableUserSubsystem_.RemoveSystemScript(tableId);
		break;
	default:
		break;
	}

	[[maybe_unused]] const bool removed = tables_.RemoveTable(tableId);
	assert(removed);

	LOG_DEBUG_FMT("Script table with ID '{}' removed", tableId);

	return true;
}

bool ScriptSystem::ContainsTable(ScriptTable::TableId tableId) const
{
	return tables_.ContainsTable(tableId);
}

Result<Void> ScriptSystem::SetTableName(ScriptTable::TableId tableId, std::string_view newName)
{
	return tables_.SetTableName(tableId, newName);
}

//ScriptTableDescriptors ScriptSystem::ExportTableDescriptors() const
//{
//	ScriptTableDescriptors descriptors{};
//	descriptors.filepaths.reserve(tableMap_.size());
//	descriptors.functionNames.reserve(tableMap_.size());
//
//	for (const auto& [_, data] : tableMap_)
//	{
//		descriptors.filepaths.emplace_back(data.filepath);
//		descriptors.functionNames.emplace_back(
//			data.table.GetFunctionSignatures() 
//			| std::views::transform([](const auto& pair) { return pair.first; }) 
//			| std::ranges::to<std::vector>()
//		);
//	}
//
//	return descriptors;
//}
