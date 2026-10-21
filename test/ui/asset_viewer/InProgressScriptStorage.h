#pragma once

#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "../../scripting/ScriptTable.h"

namespace ui {

using InProgressScriptID = uint32_t;

inline constexpr InProgressScriptID kInvalidInProgressScriptID = 
	std::numeric_limits<InProgressScriptID>::max();

namespace detail {

inline InProgressScriptID GetNextInProgressScriptID()
{
	static InProgressScriptID nextID = 0;
	return nextID++;
}

static int kScriptUniqueNameCounter = 0;

} // detail

struct CompoundScriptID
{
	ScriptTable::TableId tableId = ScriptTable::kInvalidTableId;
	InProgressScriptID inProgressId = kInvalidInProgressScriptID;

	constexpr bool Empty() const noexcept
	{
		return tableId == ScriptTable::kInvalidTableId && 
			   inProgressId == kInvalidInProgressScriptID;
	}

	constexpr bool operator==(const CompoundScriptID& other) const noexcept = default;
};

struct InProgressScriptStorage
{
	std::vector<std::string> names;
	std::vector<std::string> contents;
	std::vector<ScriptTable::TableType> tableTypes;
	std::vector<ScriptTable::TableId> tableIds;
	std::vector<InProgressScriptID> inProgressIds;

	std::unordered_map<ScriptTable::TableId, size_t> tableIdToDataIndex;
	std::unordered_map<InProgressScriptID, size_t> inProgressIdToDataIndex;

	size_t Size() const noexcept
	{
		assert(names.size() == contents.size());
		assert(names.size() == tableTypes.size());
		assert(names.size() == inProgressIds.size());
		assert(names.size() == tableIds.size());

		return names.size();
	}

	InProgressScriptID AddEntry()
	{
		assert(names.size() == contents.size());
		assert(names.size() == tableTypes.size());
		assert(names.size() == inProgressIds.size());
		assert(names.size() == tableIds.size());

		const size_t idx = names.size();
		const auto inProgressId = detail::GetNextInProgressScriptID();

		std::string uniqueName;
		if (detail::kScriptUniqueNameCounter == 0)
		{
			uniqueName = "new_script";
		}
		else
		{
			uniqueName = std::format("new_script({})", detail::kScriptUniqueNameCounter);
		}
		++detail::kScriptUniqueNameCounter;

		names.emplace_back(std::move(uniqueName));
		contents.emplace_back();
		tableTypes.emplace_back(ScriptTable::TableType::Invalid);
		tableIds.emplace_back(ScriptTable::kInvalidTableId);
		inProgressIds.emplace_back(inProgressId);

		inProgressIdToDataIndex.try_emplace(inProgressId, idx);

		return inProgressId;
	}

	bool Erase(InProgressScriptID inProgressId)
	{
		auto it = inProgressIdToDataIndex.find(inProgressId);
		if (it == inProgressIdToDataIndex.end())
		{
			return false;
		}

		const auto backIdx = Size() - 1;
		const auto toRemoveIdx = it->second;

		if (toRemoveIdx != backIdx)
		{
			const auto backInProgressId = inProgressIds[backIdx];

			assert(inProgressIdToDataIndex.contains(backInProgressId));

			std::swap(names[toRemoveIdx], names[backIdx]);
			std::swap(contents[toRemoveIdx], contents[backIdx]);
			std::swap(tableTypes[toRemoveIdx], tableTypes[backIdx]);
			std::swap(tableIds[toRemoveIdx], tableIds[backIdx]);
			std::swap(inProgressIds[toRemoveIdx], inProgressIds[backIdx]);

			inProgressIdToDataIndex[backInProgressId] = toRemoveIdx;
		}

		names.pop_back();
		contents.pop_back();
		tableTypes.pop_back();
		tableIds.pop_back();
		inProgressIds.pop_back();

		inProgressIdToDataIndex.erase(it);

		return true;
	}

	bool SetTableID(size_t dataIdx, ScriptTable::TableId tableId)
	{
		if (dataIdx >= Size())
		{
			return false;
		}

		tableIds[dataIdx] = tableId;
		tableIdToDataIndex[tableId] = dataIdx;

		return true;
	}
};


} // ui

#endif