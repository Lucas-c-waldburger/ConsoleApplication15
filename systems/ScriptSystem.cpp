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

Result<ScriptTable::TableId> ScriptSystem::AddFunctionTable(const std::string& path)
{
	TRY(tables_.AddTable(path, state_, ScriptTable::TableType::FunctionTable), table);

	assert(table);

	LOG_DEBUG_FMT("Function table script '{}' loaded successfully", 
		std::filesystem::path(path).stem().string());

	return table->GetTableId();
}

Result<ScriptTable::TableId> ScriptSystem::AddSystemTable(const std::string& path, Phase phase)
{
	TRY(tables_.AddTable(path, state_, ScriptTable::TableType::SystemTable), table);

	assert(table);

	auto result = scriptableUserSubsystem_.AddSystemScript(*table, phase);
	if (!result.Success())
	{
		tables_.RemoveTable(table->GetTableId());

		return result.GetError();
	}

	LOG_DEBUG_FMT("System table script '{}' loaded successfully", 
		std::filesystem::path(path).stem().string());

	return table->GetTableId();
}

Result<Void> ScriptSystem::ReloadTable(ScriptTable::TableId tableId)
{
	TRY(tables_.ReloadTable(tableId, state_));

	LOG_DEBUG_FMT("Script table with ID '{}' reloaded", tableId);

	return kVoid;
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

ScriptTableView ScriptSystem::GetTableView(ScriptTable::TableId tableId) const
{
	return tables_.GetTableView(tableId);
}

const std::string& ScriptSystem::GetTableFilepath(ScriptTable::TableId tableId) const
{
	return tables_.GetTableFilepath(tableId);
}

const ScriptTableDataMap& ScriptSystem::GetScriptTableMap() const
{
	return tables_.GetTableDataMap();
}

bool ScriptSystem::RemoveTable(ScriptTable::TableId tableId)
{
	if (!tables_.ContainsTable(tableId))
	{
		return false;
	}

	switch (tables_.GetTableType(tableId))
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
