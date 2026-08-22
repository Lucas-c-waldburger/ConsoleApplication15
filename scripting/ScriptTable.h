#pragma once
#include "LuaFunctionCallHandler.h"
#include "../core/Dictionary.h"

struct ScriptTableEntry;
class ScriptTableView;

class ScriptTable
{
public:
	using TableId = uint32_t;

	enum class TableType : uint8_t
	{
		Invalid = 0,
		FunctionTable,
		SystemTable
	};

	struct CallableWrapper
	{
	public:
		CallableWrapper() = default;
		CallableWrapper(sol::function&& fn, const std::vector<uint64_t>& argTypes) :
			fn_(std::move(fn)), argTypes_(argTypes) {
		}
		~CallableWrapper() = default;
		CallableWrapper(const CallableWrapper&) = delete;
		CallableWrapper& operator=(const CallableWrapper&) = delete;
		CallableWrapper(CallableWrapper&&) noexcept = delete;
		CallableWrapper& operator=(CallableWrapper&&) noexcept = delete;

		operator bool() const noexcept { return fn_.valid(); }
		bool operator!() const noexcept { return !fn_.valid(); }

		template <typename...Args>
		Result<Void> operator()(Args&&...args)
		{
			return LuaFunctionCallHandler::CallLuaFunctionQualified(
				fn_, argTypes_, std::forward<Args>(args)...);
		}

	private:
		sol::function fn_;
		std::span<const uint64_t> argTypes_;
	};

	ScriptTable() = default;
	~ScriptTable() = default;
	ScriptTable(const ScriptTable&) = delete;
	ScriptTable& operator=(const ScriptTable&) = delete;
	ScriptTable(ScriptTable&&) noexcept = default;
	ScriptTable& operator=(ScriptTable&&) noexcept = default;

	static std::pair<ScriptTable::TableId, ScriptTableEntry>
	CreateTableEntry(const std::string& path, sol::table&& tbl, 
		ParsedLuaFunctionTableSignatures&& fns, TableType type);

	bool IsValid() const noexcept
	{
		return table_.valid() && id_ != std::numeric_limits<TableId>::max();
	}

	TableId GetTableId() const noexcept { return id_; }

	ScriptTableView GetView() const;

	bool Contains(std::string_view name) const;

	CallableWrapper operator[](std::string_view name) const;

	void ReassignTableData(sol::table&& tbl, ParsedLuaFunctionTableSignatures&& fns);

	const ParsedLuaFunctionTableSignatures& GetFunctionSignatures() const { return functions_; }

	const sol::table& Data() const { return table_; }

private:
	static inline TableId tableIdCounter_ = 0;

	ScriptTable(sol::table&& tbl, ParsedLuaFunctionTableSignatures&& fns, uint32_t id) :
		table_(std::move(tbl)), functions_(std::move(fns)), id_(id) {}

	sol::table table_;
	ParsedLuaFunctionTableSignatures functions_;
	TableId id_ = std::numeric_limits<TableId>::max();
};

struct ScriptTableEntry
{
	std::string filepath;
	ScriptTable table;
	ScriptTable::TableType type = ScriptTable::TableType::Invalid;
};

using ScriptTableDataMap = std::unordered_map<ScriptTable::TableId, ScriptTableEntry>;

class ScriptTableView
{
public:
	ScriptTableView() = default;
	explicit ScriptTableView(const ScriptTable* tbl) : scriptTable_(tbl) {}

	bool Contains(std::string_view fnName) const;

	bool IsValid() const noexcept;

	ScriptTable::TableId GetTableId() const noexcept;

	ScriptTable::CallableWrapper operator[](std::string_view name) const;

	constexpr bool operator==(const ScriptTableView& rhs) const = default;

private:
	const ScriptTable* scriptTable_ = nullptr;
};