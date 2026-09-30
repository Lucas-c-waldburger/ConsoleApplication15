#include "ScriptSystem.h"
#include "../ecs/Ecs.h"
#include "../components/util/ComponentValidPreds.h"
#include <ranges>

namespace {

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
		if (sysPhase == Phase::Invalid)
		{
			return MAKE_ERROR("Script table registered as a system script, but "
				"Phase was invalid");
		}

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
	TRY(tables_.ReloadTable(tableId, state_));

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
