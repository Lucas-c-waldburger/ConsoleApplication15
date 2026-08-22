#include "ScriptTable.h"

/** @defgroup ScriptTable @{ */

std::pair<ScriptTable::TableId, ScriptTableEntry>
ScriptTable::CreateTableEntry(const std::string& path, sol::table&& tbl,
							  ParsedLuaFunctionTableSignatures&& fns, TableType type)
{
	const auto id = tableIdCounter_++;

	return std::pair<TableId, ScriptTableEntry>{
		std::piecewise_construct,
			std::forward_as_tuple(id),
			std::forward_as_tuple(path, ScriptTable{ std::move(tbl), std::move(fns), id }, type)
	};
}

ScriptTableView ScriptTable::GetView() const
{
	return ScriptTableView(this);
}

bool ScriptTable::Contains(std::string_view name) const
{
	if (functions_.contains(name))
	{
		assert(table_[name].valid());
		assert(table_[name].get_type() == sol::type::function);

		return true;
	}

	return false;
}

auto ScriptTable::operator[](std::string_view name) const -> CallableWrapper
{
	if (!IsValid())
	{
		return CallableWrapper{};
	}

	auto it = functions_.find(name);
	if (it == functions_.end())
	{
		return CallableWrapper{};
	}

	return CallableWrapper{
		table_[name].get<sol::function>(),
		it->second
	};
}

void ScriptTable::ReassignTableData(sol::table&& tbl, ParsedLuaFunctionTableSignatures&& fns)
{
	table_ = std::move(tbl);
	functions_ = std::move(fns);
}

/** @} */

/** @defgroup ScriptTableView @{ */

bool ScriptTableView::Contains(std::string_view fnName) const
{
	return IsValid() && scriptTable_->Contains(fnName);
}

bool ScriptTableView::IsValid() const noexcept
{
	return scriptTable_ && scriptTable_->IsValid();
}

ScriptTable::TableId ScriptTableView::GetTableId() const noexcept
{
	return IsValid() ? scriptTable_->GetTableId() :
					   std::numeric_limits<ScriptTable::TableId>::max();
}

ScriptTable::CallableWrapper ScriptTableView::operator[](std::string_view name) const
{
	if (IsValid())
	{
		return scriptTable_->operator[](name);
	}

	return ScriptTable::CallableWrapper{};
}

/** @} */