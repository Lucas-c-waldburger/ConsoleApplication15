#pragma once
#include "ScriptTable.h"
#include "LuaStateManager.h"
#include "../core/StableSOA.h"
#include "ScriptTableHandle.h"
#include "../systems/Phase.h"

struct ScriptTableInfo
{
	std::string name;
	std::string filepath;
	ScriptTable::TableType tableType = ScriptTable::TableType::Invalid;
	Phase systemPhase = Phase::Invalid;
	ScriptTable::TableId tableId = ScriptTable::kInvalidTableId;
};

using ScriptTableInfoSOA = StableSOA<ScriptTableInfo,
	&ScriptTableInfo::name,
	&ScriptTableInfo::filepath,
	&ScriptTableInfo::tableType,
	&ScriptTableInfo::systemPhase,
	&ScriptTableInfo::tableId
>;

struct ScriptTableDescriptor
{
	std::string name;
	std::string filepath;
	ScriptTable::TableType tableType = ScriptTable::TableType::Invalid;
	Phase systemPhase = Phase::Invalid;
};

using ScriptTableDescriptors = std::vector<ScriptTableDescriptor>;

class ScriptTableManager
{
public:
	Result<ScriptTable::TableId> AddTable(ScriptTableDescriptor&& descriptor, LuaStateManager& state);
	Result<Void> ReloadTable(ScriptTable::TableId tableId, LuaStateManager& state);
	bool RemoveTable(ScriptTable::TableId tableId);
	ScriptTable* GetTable(ScriptTable::TableId tableId);
	ScriptTableView GetTableView(ScriptTable::TableId tableId) const;
	bool ContainsTable(ScriptTable::TableId tableId) const;

	void Clear();

	Result<Void> SetTableName(ScriptTable::TableId tableId, std::string_view newName);

	auto GetTableInfo(ScriptTable::TableId tableId) const
	{
		using Ret = decltype(tableInfo_.TryGetView(0));

		auto it = tables_.find(tableId);
		if (it == tables_.end())
		{
			return Ret{ std::nullopt };
		}

		const auto& cInfo = tableInfo_;

		return cInfo.TryGetView(it->second.resourceIndex);
	}

	template <auto...MemberPtrs> requires (sizeof...(MemberPtrs) > 1)
	auto GetTableInfo(ScriptTable::TableId tableId) const
	{
		using Ret = decltype(tableInfo_.TryGetView<MemberPtrs...>(0));

		auto it = tables_.find(tableId);
		if (it == tables_.end())
		{
			return Ret{ std::nullopt };
		}

		const auto& cInfo = tableInfo_;

		return cInfo.TryGetView<MemberPtrs...>(it->second.resourceIndex);
	}

	template <auto MemberPtr>
	auto GetTableInfo(ScriptTable::TableId tableId) const
	{
		using Ret = MonoValueOptionalTupleUnwrapper<
			const typename member_ptr_traits<MemberPtr>::value_type&>;

		auto it = tables_.find(tableId);
		if (it == tables_.end())
		{
			return Ret{ std::nullopt };
		}

		const auto& cInfo = tableInfo_;

		return Ret{ cInfo.TryGetView<MemberPtr>(it->second.resourceIndex) };
	}

	template <auto...MemberPtrs> requires (sizeof...(MemberPtrs) > 0)
	auto IterTableInfo() const
	{
		return tableInfo_.ForEach<MemberPtrs...>();
	}

	ScriptTableDescriptors Serialize() const;

	static ScriptTable::TableType ResolveTableType(const ParsedLuaFunctionTableSignatures& fnSigs,
												   ScriptTable::TableType reportedTableType);

private:
	ScriptTableInfoSOA tableInfo_;
	std::unordered_map<ScriptTable::TableId, ScriptTableIndexPair> tables_;
	std::vector<size_t> freeSlots_;
};

//class ScriptTableManager
//{
//public:
//	ScriptTableManager() = default;
//	~ScriptTableManager() = default;
//	ScriptTableManager(const ScriptTableManager&) = delete;
//	ScriptTableManager& operator=(const ScriptTableManager&) = delete;
//	ScriptTableManager(ScriptTableManager&&) noexcept = delete;
//	ScriptTableManager& operator=(ScriptTableManager&&) noexcept = delete;
//
//	Result<ScriptTable*> AddTable(const std::string& path, LuaStateManager& state,
//										  ScriptTable::TableType type);
//	Result<Void> ReloadTable(ScriptTable::TableId tableId, LuaStateManager& state);
//	bool RemoveTable(ScriptTable::TableId tableId);
//	bool ContainsTable(ScriptTable::TableId tableId) const;
//	ScriptTableView GetTableView(ScriptTable::TableId tableId) const;
//	const std::string& GetTableName(ScriptTable::TableId tableId) const;
//	const std::string& GetTableFilepath(ScriptTable::TableId tableId) const;
//	ScriptTable::TableType GetTableType(ScriptTable::TableId tableId) const;
//	void Clear();
//
//	bool SetTableName(ScriptTable::TableId tableId, std::string_view newName);
//
//	const ScriptTableDataMap& GetTableDataMap() const { return tableMap_; }
//
//private:
//	ScriptTableDataMap tableMap_;
//};