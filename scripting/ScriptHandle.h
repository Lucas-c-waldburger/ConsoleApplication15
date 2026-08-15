#pragma once
#include "LuaFunctionCallHandler.h"
#include "LuaStateManager.h"

//namespace detail {
//	struct InvalidLuaType;
//	struct LuaNativeBooleanType;
//	struct LuaNativeNumberType;
//	struct LuaNativeStringType;
//} // detail
//
//static constexpr uint32_t kInvalidLuaTypeId = TypeInfo<detail::InvalidLuaType>::hash32;
//static constexpr uint32_t kNativeBooleanLuaTypeId = TypeInfo<detail::LuaNativeBooleanType>::hash32;
//static constexpr uint32_t kNativeNumberLuaTypeId = TypeInfo<detail::LuaNativeNumberType>::hash32;
//static constexpr uint32_t kNativeStringLuaTypeId = TypeInfo<detail::LuaNativeStringType>::hash32;
//
//namespace detail {
//
//template <typename T> 
//struct accepted_lua_type_qualified_traits
//{
//	using Raw = std::remove_cvref_t<T>;
//
//	static constexpr bool valid = (
//		(std::same_as<T, Raw> ||
//		 std::same_as<T, const Raw> ||
//		 std::same_as<T, Raw*> ||
//		 std::same_as<T, Raw&> ||
//		 std::same_as<T, const Raw*> ||
//		 std::same_as<T, const Raw&>) &&
//		(!HasFuncTraits<T>)
//	);
//
//	// erase distinction between Value and const Value
//	using ResolvedType = std::conditional_t<std::same_as<T, const Raw>, Raw, T>;
//};
//
//} // detail
//
//template <typename T>
//concept AcceptedLuaTypeQualified = detail::accepted_lua_type_qualified_traits<T>::valid;

struct ScriptTableEntry;
class ScriptTableView2;

class ScriptTable2
{
public:
	using TableId = uint32_t;

	struct CallableWrapper
	{
	public:
		CallableWrapper() = default;
		CallableWrapper(sol::function&& fn, const std::vector<uint64_t>& argTypes) :
			fn_(std::move(fn)), argTypes_(argTypes) {}
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
			return CallLuaFunctionQualified(fn_, argTypes_, std::forward<Args>(args)...);
		}

	private:
		sol::function fn_;
		std::span<const uint64_t> argTypes_;
	};

	ScriptTable2() = default;
	~ScriptTable2() = default;
	ScriptTable2(const ScriptTable2&) = delete;
	ScriptTable2& operator=(const ScriptTable2&) = delete;
	ScriptTable2(ScriptTable2&&) noexcept = default;
	ScriptTable2& operator=(ScriptTable2&&) noexcept = default;

	static std::pair<ScriptTable2::TableId, ScriptTableEntry>
	CreateTableEntry(const std::string& path, sol::table&& tbl, ParsedLuaFunctionTableSignatures&& fns);

	void ReassignTableData(sol::table&& tbl, ParsedLuaFunctionTableSignatures&& fns)
	{
		table_ = std::move(tbl);
		functions_ = std::move(fns);
	}

	bool IsValid() const noexcept 
	{ 
		return table_.valid() && id_ != std::numeric_limits<TableId>::max(); 
	}

	TableId GetTableId() const noexcept { return id_; }

	ScriptTableView2 GetView() const;

	bool Contains(std::string_view name) const
	{
		if (functions_.contains(name))
		{
			assert(table_[name].valid());
			assert(table_[name].get_type() == sol::type::function);

			return true;
		}

		return false;
	}

	CallableWrapper operator[](std::string_view name) const
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

	const ParsedLuaFunctionTableSignatures& GetFunctionSignatures() const { return functions_; }

private:
	static inline TableId tableIdCounter_ = 0;

	ScriptTable2(sol::table&& tbl, ParsedLuaFunctionTableSignatures&& fns, uint32_t id) :
		table_(std::move(tbl)), functions_(std::move(fns)), id_(id) {}

	sol::table table_;
	ParsedLuaFunctionTableSignatures functions_;
	TableId id_ = std::numeric_limits<TableId>::max();
};

struct ScriptTableEntry
{
	std::string filepath;
	ScriptTable2 table;
};

class ScriptTableView2
{
public:
	ScriptTableView2() = default;
	explicit ScriptTableView2(const ScriptTable2* tbl) : scriptTable_(tbl) {}

	bool Contains(std::string_view fnName) const
	{
		return IsValid() && scriptTable_->Contains(fnName);
	}

	bool IsValid() const noexcept
	{
		return scriptTable_ && scriptTable_->IsValid();
	}

	ScriptTable2::TableId GetTableId() const noexcept
	{
		return IsValid() ? scriptTable_->GetTableId() :
						   std::numeric_limits<ScriptTable2::TableId>::max();
	}

	ScriptTable2::CallableWrapper operator[](std::string_view name) const
	{
		if (IsValid())
		{
			return scriptTable_->operator[](name);
		}
		
		return ScriptTable2::CallableWrapper{};
	}

	constexpr bool operator==(const ScriptTableView2& rhs) const noexcept
	{
		return scriptTable_ == rhs.scriptTable_;
	}

private:
	friend class ScriptTable2;

	const ScriptTable2* scriptTable_ = nullptr;
};

struct ScriptTableDescriptors
{
	std::vector<std::string> filepaths;
	std::vector<ParsedLuaFunctionTableSignatures> tableFunctions;
};

class ScriptSystem2
{
public: 
	Result<ScriptTable2::TableId> AddTable(const std::string& path)
	{
		auto loadResult = state_.LoadScriptFile(path);
		if (!loadResult.valid())
		{
			sol::error err = loadResult;
			return MAKE_ERROR(err.what());
		}

		if (loadResult.get_type() != sol::type::table)
		{
			return MAKE_ERROR_FMT("Script at path '{}' did not return a sol::table", path);
		}

		auto table = loadResult.get<sol::table>();

		TRY(LuaFunctionTableParser::ParseLuaFunctionTableSignatures(table, state_), parsed);

		const auto it = tableMap_.emplace(
			ScriptTable2::CreateTableEntry(path, std::move(table), std::move(parsed))).first;

		return it->first;
	}

	Result<Void> ReloadTable(ScriptTable2::TableId tableId)
	{
		auto it = tableMap_.find(tableId);
		if (it == tableMap_.end())
		{
			return MAKE_ERROR("Table with provided Id not found");
		}

		auto loadResult = state_.LoadScriptFile(it->second.filepath);
		if (!loadResult.valid())
		{
			sol::error err = loadResult;
			return MAKE_ERROR(err.what());
		}

		if (loadResult.get_type() != sol::type::table)
		{
			return MAKE_ERROR_FMT("Script at path '{}' did not return a sol::table", 
				it->second.filepath);
		}

		auto table = loadResult.get<sol::table>();

		TRY(LuaFunctionTableParser::ParseLuaFunctionTableSignatures(table, state_), parsed);

		it->second.table.ReassignTableData(std::move(table), std::move(parsed));

		return kVoid;
	}

	ScriptTableView2 GetTableView(ScriptTable2::TableId tableId) const
	{
		auto it = tableMap_.find(tableId);
		
		return (it != tableMap_.end()) ? ScriptTableView2{ &it->second.table } : 
										 ScriptTableView2{};
	}

	ScriptTableDescriptors ExportTableDescriptors() const
	{
		ScriptTableDescriptors descriptors;
		descriptors.filepaths.reserve(tableMap_.size());
		descriptors.tableFunctions.reserve(tableMap_.size());

		for (const auto& [_, data] : tableMap_)
		{
			descriptors.filepaths.emplace_back(data.filepath);
			descriptors.tableFunctions.emplace_back(data.table.GetFunctionSignatures());
		}

		return descriptors;
	}

	Result<Void> LoadTables(ScriptTableDescriptors&& descriptors)
	{
		assert(descriptors.filepaths.size() == descriptors.tableFunctions.size());
		for (size_t i = 0; i < descriptors.filepaths.size(); ++i)
		{
			auto loadResult = state_.LoadScriptFile(descriptors.filepaths[i]);
			if (!loadResult.valid())
			{
				sol::error err = loadResult;
				return MAKE_ERROR(err.what());
			}

			if (loadResult.get_type() != sol::type::table)
			{
				return MAKE_ERROR_FMT("Script at path '{}' did not return a sol::table", 
					descriptors.filepaths[i]);
			}

			auto table = loadResult.get<sol::table>();

			tableMap_.emplace(ScriptTable2::CreateTableEntry(
				descriptors.filepaths[i], std::move(table),
				std::move(descriptors.tableFunctions[i])));
		}

		return kVoid;
	}

	LuaStateManager& GetState() { return state_; }
	const LuaStateManager& GetState() const { return state_; }

private:
	using ScriptTableMap = std::unordered_map<ScriptTable2::TableId, ScriptTableEntry>;

	//Result<ParsedLuaFunctionTableSignatures>
	//ParseLuaFunctionTableSignatures(sol::table& fnTable)
	//{
	//	sol::table meta = fnTable[sol::metatable_key];
	//	if (!meta.valid())
	//	{
	//		return MAKE_ERROR("No metatable found in module");
	//	}

	//	sol::table signatures = meta["__signatures"];
	//	if (!signatures.valid())
	//	{
	//		return MAKE_ERROR("No field '__signatures' found in metatable");
	//	}

	//	ParsedLuaFunctionTableSignatures parsed;

	//	for (auto&& [fnName, fn] : fnTable)
	//	{
	//		if (fnName.get_type() != sol::type::string)
	//		{
	//			continue;
	//		}

	//		const auto fnNameStr = fnName.as<std::string>();

	//		if (!fn.is<sol::function>())
	//		{
	//			LOG_DEBUG_FMT("Table field '{}' is not a function", fnNameStr);
	//			continue;
	//		}

	//		sol::table fnSig = signatures[fnNameStr];
	//		if (!fnSig.valid())
	//		{
	//			return MAKE_ERROR_FMT("Table function '{}' did not have a matching metatable signature",
	//				fnNameStr);
	//		}

	//		auto [it, fnNameInserted] = parsed.try_emplace(fnNameStr, std::vector<uint64_t>{});
	//		assert(fnNameInserted);

	//		auto& argTypeIds = it->second;
	//		argTypeIds.reserve(fnSig.size());

	//		for (auto&& [_, arg] : fnSig)
	//		{
	//			if (arg.get_type() != sol::type::string)
	//			{
	//				return MAKE_ERROR("Argument name was not of type string");
	//			}

	//			const auto argStr = arg.as<std::string>();

	//			TRY_ASSIGN(argTypeIds.emplace_back(), ParseFullArgumentType(argStr));
	//		}
	//	}

	//	return parsed;
	//}

	//Result<uint64_t> ParseFullArgumentType(const std::string& argStr)
	//{
	//	static constexpr auto regex = ctll::fixed_string{ R"(
	//		^\s*(?:(?<leading_const>const)\s+)?(?<type>[A-Za-z_][A-Za-z0-9_]*)
	//		(?:\s+(?<trailing_const>const))?\s*(?<qualifier>[&*])?\s*$
	//	)" };

	//	auto match = ctre::match<regex>(argStr);
	//	if (!match)
	//	{
	//		return MAKE_ERROR_FMT("Argument string '{}' did not match regex", argStr);
	//	}

	//	uint32_t typeId = kInvalidLuaTypeId;
	//	uint32_t qualifiers = 0;

	//	auto typeName = match.get<"type">().to_view();
	//	if (typeName.empty())
	//	{
	//		return MAKE_ERROR_FMT("Argument string '{}' missing type name", argStr);
	//	}

	//	if (!state_.IsRegistered(typeName))
	//	{
	//		return MAKE_ERROR_FMT("Argument type '{}' not registered with lua state", typeName);
	//	}

	//	typeId = state_.GetRegisteredTypeId(typeName);
	//	assert(typeId != kInvalidLuaTypeId);

	//	const auto qual = match.get<"qualifier">().to_view();

	//	qualifiers |= (qual == "&" ? LuaTypeQualifiers::Ref :
	//				   qual == "*" ? LuaTypeQualifiers::Ptr : 0);

	//	const bool isRefOrPtr = (qualifiers & (LuaTypeQualifiers::Ref | LuaTypeQualifiers::Ptr)) != 0;
	//	if (isRefOrPtr)
	//	{
	//		if (IsNativeLuaType(typeName))
	//		{
	//			LOG_WARNING_FMT("Native lua type argument '{}' was marked as a reference or pointer, "
	//				"but native types can only be passed by value. Consider wrapping the argument in "
	//				"a user type to get reference behavior", typeName);

	//			qualifiers = 0;
	//		}
	//		else
	//		{
	//			// only add const if type is not value-only
	//			const bool isConst = !match.get<"leading_const">().to_view().empty() ||
	//								 !match.get<"trailing_const">().to_view().empty();
	//			if (isConst)
	//			{
	//				qualifiers |= LuaTypeQualifiers::Const;
	//			}
	//		}
	//	}

	//	return uint64_t{
	//		(static_cast<uint64_t>(qualifiers) << 32) | static_cast<uint64_t>(typeId)
	//	};
	//}

	LuaStateManager state_;
	ScriptTableMap tableMap_;
};