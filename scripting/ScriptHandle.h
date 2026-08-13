#pragma once
#include "LuaUserType.h"
#include "../core/Dictionary.h"
#include "../core/TypeInfo.h"
#include "../core/Handle.h"
#include "../core/FuncTraits.h"
#include "../deps/ctre/ctre.hpp"
#include <limits>
#include <cassert>
#include <span>

//template<typename T>
//std::string GetSolTypeString(sol::state_view state) 
//{
//    const sol::type t = sol::type_of<T>();
//
//    switch (t) 
//    {
//    case sol::type::userdata:
//    {
//        // return the name for the actual type of this userdata as registered during .new_usertype
//    }
//    case sol::type::number:   return "number";
//    case sol::type::string:   return "string";
//    case sol::type::boolean:  return "boolean";
//    case sol::type::table:    return "table";
//    case sol::type::function: return "function";
//    default:                  return "unknown";
//    }
//}

//std::string GetSolObjectUserDataName(sol::state_view state, const sol::object& obj)
//{
//    if (obj.get_type() != sol::type::userdata)
//    {
//        return {};
//    }
//
//    lua_State* L = state.lua_state();
//
//    obj.push();
//
//    if (!lua_getmetatable(L, -1))
//    {
//        lua_pop(L, 1);
//        return {};
//    }
//
//    lua_getfield(L, -1, "__type");
//
//    if (!lua_istable(L, -1))
//    {
//        lua_settop(L, -3);
//        return {};
//    }
//
//    lua_getfield(L, -1, "name");
//
//    if (!lua_isstring(L, -1))
//    {
//        lua_settop(L, -4);
//        return {};
//    }
//
//    std::string name = lua_tostring(L, -1);
//
//    lua_settop(L, -4);
//    return name;
//}

//template <typename T>
//inline constexpr std::string_view GetLuaMetadataTypeName()
//{
//	using Type = std::remove_cvref_t<T>;
//
//	if constexpr (SomeLuaUserType<T>)
//	{
//		return lua_user_type_name<T>::value;
//	}
//	else if constexpr (std::same_as<T, bool>)
//	{
//		return "boolean";
//	}
//	else if constexpr (std::is_arithmetic_v<T>)
//	{
//		return "number";
//	}
//	else if constexpr (std::constructible_from<std::string, T>)
//	{
//		return "string";
//	}
//
//	sol::type t;
//	switch (t)
//	{
//    case sol::type::userdata: return GetSolObjectUserDataName(state, obj);
//	case sol::type::number:   return "number";
//	case sol::type::string:   return "string";
//	case sol::type::boolean:  return "boolean";
//	case sol::type::table:    return "table";
//	case sol::type::function: return "function";
//	default:                  return "unknown";
//	}
//}
//
//template <typename...Args>
//void f(sol::table& luaModule, std::string_view fnToCall, Args&&...args)
//{
//	sol::optional<sol::function> tableFn = luaModule[fnToCall];
//	if (!tableFn)
//	{
//		return;
//	}
//
//	sol::table meta = luaModule[sol::metatable_key];
//	if (!meta.valid())
//	{
//		LOG_ERROR("No metatable found in module");
//		return;
//	}
//
//	sol::table sigs = meta["__signatures"];
//	if (!sigs.valid())
//	{
//		LOG_ERROR("No field '__signatures' found in metatable");
//		return;
//	}
//
//	for (auto&& [fnName, sigList] : sigs)
//	{
//
//	}
//
//	auto argTup = std::forward_as_tuple(std::forward<Args>(args)...);
//}

static constexpr uint32_t kInvalidLuaTypeId = 0;
static constexpr uint32_t kNativeBooleanLuaTypeId = 1;
static constexpr uint32_t kNativeNumberLuaTypeId = 2;
static constexpr uint32_t kNativeStringLuaTypeId = 3;

template <typename T> requires (std::is_class_v<T> || std::is_enum_v<T>)
uint32_t GetLuaTypeId();

namespace detail {
struct LuaTypeIdImpl
{
private:
	template <typename T> requires (std::is_class_v<T> || std::is_enum_v<T>)
	friend uint32_t GetLuaTypeId();

	static inline uint32_t value_ = 4;
};
} // detail

template <typename T> requires (std::is_class_v<T> || std::is_enum_v<T>)
inline uint32_t GetLuaTypeId()
{
	static const uint32_t id = detail::LuaTypeIdImpl::value_++;
	return id;
}

namespace detail {

template <typename T> 
struct accepted_lua_type_qualified_traits
{
	using Raw = std::remove_cvref_t<T>;

	static constexpr bool valid = (
		(std::same_as<T, Raw> ||
		 std::same_as<T, const Raw> ||
		 std::same_as<T, Raw*> ||
		 std::same_as<T, Raw&> ||
		 std::same_as<T, const Raw*> ||
		 std::same_as<T, const Raw&>) &&
		(!HasFuncTraits<T>)
	);

	// erase distinction between Value and const Value
	using ResolvedType = std::conditional_t<std::same_as<T, const Raw>, Raw, T>;
};

} // detail

template <typename T>
concept AcceptedLuaTypeQualified = detail::accepted_lua_type_qualified_traits<T>::valid;

enum LuaTypeQualifiers : uint8_t
{
	Const = 1 << 0,
	Ref = 1 << 1,
	Ptr = 1 << 2
};

class LuaTypeRegistry
{
private:
	struct InvalidLuaType;
	struct LuaNativeBooleanType;
	struct LuaNativeNumberType;
	struct LuaNativeStringType;

public:
	template <typename T>
	static bool RegisterType(std::string_view name)
	{
		if (name.empty())
		{
			return false;
		}

		return RegisterImpl<T>(name);
	}

	template <typename T>
	static bool IsTypeRegistered()
	{
		using Type = raw_type_t<T>;

		if constexpr (IsNativeLuaType<Type>())
		{
			return true;
		}
		else if constexpr (std::is_class_v<Type> || std::is_enum_v<Type>)
		{
			return typeIdToName_.contains(GetLuaTypeId<Type>());
		}
		else
		{
			return false;
		}
	}

	static bool IsTypeRegistered(std::string_view name)
	{
		if (IsNativeLuaType(name))
		{
			return true;
		}

		return nameToTypeId_.contains(name);
	}

	template <typename T>
	static std::string_view GetTypeName()
	{
		using Type = raw_type_t<T>;

		if constexpr (IsNativeLuaType<Type>)
		{
			return GetNativeLuaTypeName<Type>();
		}
		else if constexpr (std::is_class_v<Type> || std::is_enum_v<Type>)
		{
			return GetAssignedTypeName<Type>();
		}
		else
		{
			return {};
		}
	}

	static uint64_t GetTypeId(std::string_view name)
	{
		if (IsNativeLuaType(name))
		{
			return name == "boolean" ? TypeInfo<LuaNativeBooleanType>::hash :
				   name == "number"  ? TypeInfo<LuaNativeNumberType>::hash : 
									   TypeInfo<LuaNativeStringType>::hash;
		}

		auto it = nameToTypeId_.find(name);

		return (it != nameToTypeId_.end()) ? it->second : TypeInfo<InvalidLuaType>::hash;
	}

	template <typename T>
	static uint64_t GetTypeId()
	{
		using Type = raw_type_t<T>;

		if constexpr (IsNativeLuaType<Type>())
		{
			return GetNativeLuaTypeId<Type>();
		}
		else if constexpr (std::is_class_v<Type> || std::is_enum_v<Type>)
		{
			constexpr auto typeId = TypeInfo<Type>::hash;

			return (typeIdToName_.contains(typeId)) 
				? typeId 
				: TypeInfo<InvalidLuaType>::hash;
		}
		else
		{
			return kInvalidLuaTypeId;
		}
	}

	static constexpr bool IsNativeLuaType(std::string_view name) noexcept
	{
		return name == "boolean" || name == "number" || name == "string";
	}

	template <typename T>
	static constexpr bool IsNativeLuaType() noexcept
	{
		using Raw = raw_type_t<T>;

		if constexpr (std::same_as<Raw, bool>)
		{
			return true;
		}
		else if constexpr (std::is_arithmetic_v<Raw> && !std::same_as<Raw, char>)
		{
			return true;
		}
		else if constexpr (std::constructible_from<std::string, Raw> || std::same_as<Raw, char>)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	template <typename T>
	static constexpr uint64_t GetNativeLuaTypeId() noexcept
	{
		using Raw = raw_type_t<T>;

		if constexpr (std::same_as<Raw, bool>)
		{
			return TypeInfo<LuaNativeBooleanType>::hash;
		}
		else if constexpr (std::is_arithmetic_v<Raw> && !std::same_as<Raw, char>)
		{
			return TypeInfo<LuaNativeNumberType>::hash;
		}
		else if constexpr (std::constructible_from<std::string, Raw> || 
						   std::same_as<Raw, char>)
		{
			return TypeInfo<LuaNativeStringType>::hash;
		}
		else
		{
			return TypeInfo<InvalidLuaType>::hash;
		}
	}

	template <typename T>
	static constexpr std::string_view GetNativeLuaTypeName() noexcept
	{
		using Raw = raw_type_t<T>;

		if constexpr (std::same_as<Raw, bool>)
		{
			return "boolean";
		}
		else if constexpr (std::is_arithmetic_v<Raw> && !std::same_as<Raw, char>)
		{
			return "number";
		}
		else if constexpr (std::constructible_from<std::string, Raw> || std::same_as<Raw, char>)
		{
			return "string"
		}
		else
		{
			return "";
		}
	}

private:
	template <typename T>
	static const std::string& GetAssignedTypeName(std::string_view name)
	{
		static const std::string kTypeName{ name };

		return kTypeName;
	}

	template <typename T>
	static bool RegisterImpl(std::string_view name)
	{
		using Type = raw_type_t<T>;

		if constexpr (IsNativeLuaType<Type>())
		{
			return false;
		}
		else if constexpr (std::is_class_v<Type> || std::is_enum_v<Type>)
		{
			const std::string_view assignedName = GetAssignedTypeName<T>(name);
			if (assignedName != name)
			{
				LOG_ERROR_FMT("Type '{}' already registered with global name '{}'",
					name, assignedName);

				return false;
			}

			if (nameToTypeId_.contains(assignedName))
			{
				return true;
			}

			constexpr auto typeId = TypeInfo<Type>::hash;

			auto [it, insertedName] = nameToTypeId_.try_emplace(assignedName, typeId);
			assert(insertedName);

			auto [_, insertedType] = typeIdToName_.try_emplace(typeId, assignedName);
			assert(insertedType);

			return true;
		}
		else
		{
			return false;
		}
	}

	static std::unordered_map<std::string_view, uint64_t> nameToTypeId_;
	static std::unordered_map<uint64_t, std::string_view> typeIdToName_;
};

struct ParsedLuaArgumentType
{
	uint64_t typeId = 0;
	uint8_t qualifiers = 0;
};

using ParsedLuaFunctionTableSignatures = 
	UnorderedDictionary<std::vector<ParsedLuaArgumentType>>;

inline Result<ParsedLuaArgumentType> ParseFullArgumentType(const std::string& argStr)
{
	static constexpr auto regex = ctll::fixed_string{ R"(
			^\s*(?:(?<leading_const>const)\s+)?(?<type>[A-Za-z_][A-Za-z0-9_]*)
			(?:\s+(?<trailing_const>const))?\s*(?<qualifier>[&*])?\s*$
		)" };

	auto match = ctre::match<regex>(argStr);
	if (!match)
	{
		return MAKE_ERROR_FMT("Argument string '{}' did not match regex", argStr);
	}

	ParsedLuaArgumentType parsed{};

	auto typeName = match.get<"type">().to_view();
	if (typeName.empty())
	{
		return MAKE_ERROR_FMT("Argument string '{}' missing type name", argStr);
	}

	parsed.typeId = LuaTypeRegistry::GetTypeId(typeName);
	if (parsed.typeId == 0)
	{
		return MAKE_ERROR_FMT("Argument type '{}' not registered", typeName);
	}

	if (!LuaTypeRegistry::IsNativeLuaType(typeName))
	{
		const auto qual = match.get<"qualifier">().to_view();

		parsed.qualifiers |= (qual == "&" ? LuaTypeQualifiers::Ref :
							  qual == "*" ? LuaTypeQualifiers::Ptr : 0);

		// only add const if type is not value-only
		if ((parsed.qualifiers & (LuaTypeQualifiers::Ref | LuaTypeQualifiers::Ptr)) != 0)
		{
			const bool isConst = !match.get<"leading_const">().to_view().empty() ||
								 !match.get<"trailing_const">().to_view().empty();

			if (isConst)
			{
				parsed.qualifiers |= LuaTypeQualifiers::Const;
			}
		}
	}

	return parsed;
}

//Result<ParsedLuaFunctionTableSignatures>
//inline ParseLuaFunctionTableSignatures(sol::table& fnTable, LuaTypeRegistry& registry)
//{
//	sol::table meta = fnTable[sol::metatable_key];
//	if (!meta.valid())
//	{
//		return MAKE_ERROR("No metatable found in module");
//	}
//
//	sol::table signatures = meta["__signatures"];
//	if (!signatures.valid())
//	{
//		return MAKE_ERROR("No field '__signatures' found in metatable");
//	}
//
//	ParsedLuaFunctionTableSignatures parsed;
//
//	for (auto&& [fnName, fn] : fnTable)
//	{
//		if (fnName.get_type() != sol::type::string)
//		{
//			continue;
//		}
//
//		const auto fnNameStr = fnName.as<std::string>();
//
//		if (!fn.is<sol::function>())
//		{
//			LOG_DEBUG_FMT("Table field '{}' is not a function", fnNameStr);
//			continue;
//		}
//
//		sol::table fnSig = signatures[fnName];
//		if (!fnSig.valid())
//		{
//			return MAKE_ERROR_FMT("Table function '{}' did not have a metatable signature",
//				fnNameStr);
//		}
//
//		auto [it, fnNameInserted] = parsed.try_emplace(fnNameStr, std::vector<uint64_t>{});
//		assert(fnNameInserted);
//
//		auto& argTypeIds = it->second;
//		argTypeIds.reserve(fnSig.size());
//
//		for (auto&& [_, arg] : fnSig)
//		{
//			if (arg.get_type() != sol::type::string)
//			{
//				return MAKE_ERROR("Signature argument was not of type string");
//			}
//
//			const auto argStr = arg.as<std::string>();
//
//			TRY_ASSIGN(argTypeIds.emplace_back(), ParseFullArgumentType(argStr, registry));
//		}
//	}
//
//	return parsed;
//}

template <typename T>
inline sol::object MakeArgumentLuaObject(sol::state_view state, T&& value)
{
	if constexpr (std::is_lvalue_reference_v<T> && !LuaTypeRegistry::IsNativeLuaType<T>())
	{
		if constexpr (std::is_const_v<std::remove_reference_t<T>>)
		{
			return sol::make_object(state, std::cref(value));
		}
		else
		{
			return sol::make_object(state, std::ref(value));
		}
	}
	else
	{
		return sol::make_object(state, std::forward<T>(value));
	}
}

constexpr bool IsParsedArgPointer(uint64_t parsedArg)
{
	return ((parsedArg >> 32) & static_cast<uint64_t>(LuaTypeQualifiers::Ptr)) != 0;
}

template <size_t I, size_t N, typename Tup>
void MakeArgumentLuaObjectsImpl(sol::state_view state, const std::vector<uint64_t>& parsedArgTypes,
								std::array<uint64_t, N>& normalizedArgTypes, size_t parsedArgsCurrentIdx, 
								std::vector<sol::object>& results, Tup&& tup, size_t& failedAtIdx)
{
	if (failedAtIdx < parsedArgTypes.size() || parsedArgsCurrentIdx >= parsedArgTypes.size())
	{
		return;
	}

	if constexpr (I >= N)
	{
		// couldn't match arg, check if its a pointer that we can pass 'nil' to
		if (IsParsedArgPointer(parsedArgTypes[parsedArgsCurrentIdx]))
		{
			LOG_WARNING_FMT("Could not match argument at position '{}', passed nil instead",
				parsedArgsCurrentIdx);

			results.emplace_back();
		}
		else
		{
			failedAtIdx = parsedArgsCurrentIdx;

			return;
		}
		
		if (parsedArgsCurrentIdx + 1 < parsedArgTypes.size())
		{
			MakeArgumentLuaObjectsImpl<0>(state, parsedArgTypes, normalizedArgTypes,
				parsedArgsCurrentIdx + 1, results, tup, failedAtIdx);
		}

		return;
	}

	if (normalizedArgTypes[I] != kInvalidLuaTypeId && // argument not already consumed
		parsedArgTypes[parsedArgsCurrentIdx] == normalizedArgTypes[I])
	{
		results.emplace_back(MakeArgumentLuaObject(state, std::get<I>(tup)));

		normalizedArgTypes[I] = kInvalidLuaTypeId;

		MakeArgumentLuaObjectsImpl<0>(state, parsedArgTypes, normalizedArgTypes,
			parsedArgsCurrentIdx + 1, results, tup, failedAtIdx);

		return;
	}

	MakeArgumentLuaObjectsImpl<I + 1>(state, parsedArgTypes, normalizedArgTypes,
		parsedArgsCurrentIdx, results, tup, failedAtIdx);
}

template <size_t N, typename...Args>
inline Result<std::vector<sol::object>> 
MakeArgumentLuaObjects(sol::state_view state, std::span<const uint64_t> parsedArgTypes,
					   std::array<uint64_t, N>& normalizedArgTypes, Args&&...args)
{
	auto tup = std::forward_as_tuple(std::forward<Args>(args)...);

	std::vector<sol::object> results;
	results.reserve(parsedArgTypes.size());

	size_t failedAtIdx = std::numeric_limits<size_t>::max();

	MakeArgumentLuaObjectsImpl<0>(state, parsedArgTypes, normalizedArgTypes, 
								  0, results, tup, failedAtIdx);

	if (failedAtIdx < parsedArgTypes.size())
	{
		return MAKE_ERROR_FMT("Could not match argument at position '{}'", failedAtIdx);
	}

	return results;
}

template <AcceptedLuaTypeQualified T>
inline uint64_t NormalizeArgument()
{
	using Type = typename detail::accepted_lua_type_qualified_traits<T>::ResolvedType;
	using Raw = raw_type_t<Type>;

	if constexpr (LuaTypeRegistry::IsNativeLuaType<Raw>())
	{
		return static_cast<uint64_t>(LuaTypeRegistry::GetNativeLuaTypeId<Raw>());
	}
	else
	{
		uint32_t typeId = LuaTypeRegistry::GetTypeId<Raw>();
		if (typeId == 0)
		{
			return 0;
		}

		uint32_t qualifiers = 0;

		using StripRefAndPtr = std::remove_pointer_t<std::remove_reference_t<Type>>;
		if constexpr (std::is_const_v<StripRefAndPtr>)
		{
			qualifiers |= LuaTypeQualifiers::Const;
		}

		using StripConst = std::remove_const_t<Type>;
		if constexpr (std::is_reference_v<StripConst>)
		{
			qualifiers |= LuaTypeQualifiers::Ref;
		}
		else if constexpr (std::is_pointer_v<Type>)
		{
			qualifiers |= LuaTypeQualifiers::Ptr;
		}

		return uint64_t{
			(static_cast<uint64_t>(qualifiers) << 32 | static_cast<uint64_t>(typeId))
		};
	}
}

template <typename...Args>
inline std::array<uint64_t, sizeof...(Args)> NormalizeArguments()
{
	return { NormalizeArgument<Args>()... };
}

template <typename...Args>
inline Result<Void> CallLuaFunctionQualified(sol::function& fn,
											 std::span<const uint64_t> parsedArgTypes,
											 Args&&...args)
{
	//if (!fn.valid())
	//{
	//	return MAKE_ERROR("sol function was invalid");
	//}
	assert(fn.valid());

	if (parsedArgTypes.size() > sizeof...(Args))
	{
		return MAKE_ERROR_FMT("Lua function requires {} arguments, only {} arguments provided",
			parsedArgTypes.size(), sizeof...(Args));
	}
	if (parsedArgTypes.size() < sizeof...(Args))
	{
		LOG_WARNING_FMT("Lua function requires only {} arguments, {} arguments provided",
			parsedArgTypes.size(), sizeof...(Args));
	}

	auto normalizedArgs = NormalizeArguments<Args...>();

	TRY(MakeArgumentLuaObjects(fn.lua_state(), parsedArgTypes, normalizedArgs,
		std::forward<Args>(args)...), argObjects);

	sol::protected_function_result result = fn(sol::as_args(argObjects));
	if (!result.valid())
	{
		sol::error err = result;
		return MAKE_ERROR_FMT("Failed to call lua function: '{}'", err.what());
	}

	return kVoid;
}

class LuaStateManager
{
public:
	template <SomeLuaUserType...Ts>
	bool InitWithEngineTypes()
	{
		state_.open_libraries(sol::lib::base);

		((LuaUserType<Ts>::Register(state_)), ...);
	}

	template <typename T, typename...Args> requires std::is_class_v<T>
	bool NewUserType(std::string_view name, Args&&...args)
	{
		using Type = raw_type_t<T>;

		if (state_[name] != sol::type::nil)
		{
			return false;
		}

		if (!LuaTypeRegistry::RegisterType<Type>(name))
		{
			return false;
		}

		state_.new_usertype<Type>(name, std::forward<Args>(args)...);

		return true;
	}

	template <typename T, typename...Args> requires std::is_enum_v<T>
	bool NewEnum(std::string_view name, Args&&...args)
	{
		using Type = raw_type_t<T>;

		if (state_[name] != sol::type::nil)
		{
			return false;
		}

		if (!LuaTypeRegistry::RegisterType<Type>(name))
		{
			return false;
		}

		state_.new_enum(name, std::forward<Args>(args)...);

		return true;
	}

	sol::protected_function_result LoadScriptFile(const std::string& path)
	{
		return state_.script_file(path);
	}

	sol::protected_function_result LoadScriptString(const std::string& str)
	{
		return state_.script(str);
	}

	bool Contains(std::string_view name) const
	{
		if (LuaTypeRegistry::IsNativeLuaType(name))
		{
			return true;
		}

		return LuaTypeRegistry::IsTypeRegistered(name) && state_[name] != sol::type::nil;
	}

private:
	sol::state state_;
};

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
		CallableWrapper(sol::function&& fn, const std::vector<ParsedLuaArgumentType>& argTypes) :
			fn_(std::move(fn)), argTypes_(argTypes) {}
		~CallableWrapper() = default;
		CallableWrapper(const CallableWrapper&) = delete;
		CallableWrapper& operator=(const CallableWrapper&) = delete;
		CallableWrapper(CallableWrapper&&) noexcept = default;
		CallableWrapper& operator=(CallableWrapper&&) noexcept = default;

		operator bool() const noexcept { return fn_.valid(); }
		bool operator!() const noexcept { return !fn_.valid(); }

		template <typename...Args>
		Result<Void> operator()(Args&&...args)
		{
			return CallLuaFunctionQualified(fn_, args, std::forward<Args>(args)...);
		}

	private:
		sol::function fn_;
		std::span<const ParsedLuaArgumentType> argTypes_;
	};

	ScriptTable2() = default;
	~ScriptTable2() = default;
	ScriptTable2(const ScriptTable2&) = delete;
	ScriptTable2& operator=(const ScriptTable2&) = delete;
	ScriptTable2(ScriptTable2&&) noexcept = default;
	ScriptTable2& operator=(ScriptTable2&&) noexcept = default;

	static std::pair<TableId, ScriptTableEntry>
	CreateTableEntry(const std::string& path, sol::table&& tbl, ParsedLuaFunctionTableSignatures&& fns);

	void ReassignLuaData(sol::table&& tbl, ParsedLuaFunctionTableSignatures&& fns)
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

	//template <typename...Args>
	//Result<Void> CallFunction(std::string_view name, Args&&...args) const
	//{
	//	auto it = functions_.find(name);
	//	if (it == functions_.end())
	//	{
	//		return MAKE_ERROR_FMT("Script table did not contain function '{}'", name);
	//	}

	//	sol::function fn = table_[name];
	//	assert(fn.valid());

	//	return CallLuaFunctionQualified(fn, it->second, std::forward<Args>(args)...);
	//}

	CallableWrapper operator[](std::string_view name) const
	{
		auto it = functions_.find(name);

		return (it != functions_.end()) 
			? CallableWrapper{ table_[name].get<sol::function>(), it->second }
			: CallableWrapper{};
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
	explicit ScriptTableView2(const ScriptTable2& tbl) : scriptTable_(&tbl) {}

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

	//template <typename...Args>
	//Result<Void> CallFunction(std::string_view name, Args&&...args) const
	//{
	//	if (IsValid())
	//	{
	//		return scriptTable_->CallFunction(name, std::forward<Args>(args)...);
	//	}

	//	return MAKE_ERROR("Script table was null");
	//}

	ScriptTable2::CallableWrapper operator[](std::string_view name) const
	{
		return IsValid() ? scriptTable_->operator[](name) : ScriptTable2::CallableWrapper{};
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

		TRY(ParseLuaFunctionTableSignatures(table), parsed);

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

		TRY(ParseLuaFunctionTableSignatures(table), parsed);

		it->second.table.ReassignLuaData(std::move(table), std::move(parsed));

		return kVoid;
	}

	ScriptTableView2 GetTableView(ScriptTable2::TableId tableId) const
	{
		auto it = tableMap_.find(tableId);
		
		return (it != tableMap_.end()) ? ScriptTableView2{ it->second.table } : 
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

private:
	using ScriptTableMap = std::unordered_map<ScriptTable2::TableId, ScriptTableEntry>;

	Result<ParsedLuaFunctionTableSignatures>
	ParseLuaFunctionTableSignatures(sol::table& fnTable)
	{
		sol::table meta = fnTable[sol::metatable_key];
		if (!meta.valid())
		{
			return MAKE_ERROR("No metatable found in module");
		}

		sol::table signatures = meta["__signatures"];
		if (!signatures.valid())
		{
			return MAKE_ERROR("No field '__signatures' found in metatable");
		}

		ParsedLuaFunctionTableSignatures parsed;

		for (auto&& [fnName, fn] : fnTable)
		{
			if (fnName.get_type() != sol::type::string)
			{
				continue;
			}

			const auto fnNameStr = fnName.as<std::string>();

			if (!fn.is<sol::function>())
			{
				LOG_DEBUG_FMT("Table field '{}' is not a function", fnNameStr);
				continue;
			}

			sol::table fnSig = signatures[fnName];
			if (!fnSig.valid())
			{
				return MAKE_ERROR_FMT("Table function '{}' did not have a matching metatable signature",
					fnNameStr);
			}

			auto [it, fnNameInserted] = parsed.try_emplace(fnNameStr, std::vector<uint64_t>{});
			assert(fnNameInserted);

			auto& argTypeIds = it->second;
			argTypeIds.reserve(fnSig.size());

			for (auto&& [_, arg] : fnSig)
			{
				if (arg.get_type() != sol::type::string)
				{
					return MAKE_ERROR("argument name was not of type string");
				}

				const auto argStr = arg.as<std::string>();

				TRY_ASSIGN(argTypeIds.emplace_back(), ParseFullArgumentType(argStr));
			}
		}

		return parsed;
	}

	Result<ParsedLuaArgumentType> ParseFullArgumentType(const std::string& argStr)
	{
		static constexpr auto regex = ctll::fixed_string{ R"(
			^\s*(?:(?<leading_const>const)\s+)?(?<type>[A-Za-z_][A-Za-z0-9_]*)
			(?:\s+(?<trailing_const>const))?\s*(?<qualifier>[&*])?\s*$
		)" };

		auto match = ctre::match<regex>(argStr);
		if (!match)
		{
			return MAKE_ERROR_FMT("Argument string '{}' did not match regex", argStr);
		}

		ParsedLuaArgumentType parsed{};

		auto typeName = match.get<"type">().to_view();
		if (typeName.empty())
		{
			return MAKE_ERROR_FMT("Argument string '{}' missing type name", argStr);
		}

		if (!state_.Contains(typeName))
		{
			return MAKE_ERROR_FMT("Argument type '{}' not found in state", typeName);
		}

		parsed.typeId = LuaTypeRegistry::GetTypeId(typeName);
		assert(parsed.typeId != 0);

		if (!LuaTypeRegistry::IsNativeLuaType(typeName))
		{
			const auto qual = match.get<"qualifier">().to_view();

			parsed.qualifiers |= (qual == "&" ? LuaTypeQualifiers::Ref :
								  qual == "*" ? LuaTypeQualifiers::Ptr : 0);

			// only add const if type is not value-only
			if ((parsed.qualifiers & (LuaTypeQualifiers::Ref | LuaTypeQualifiers::Ptr)) 
				!= 0)
			{
				const bool isConst = !match.get<"leading_const">().to_view().empty() ||
									 !match.get<"trailing_const">().to_view().empty();

				if (isConst)
				{
					parsed.qualifiers |= LuaTypeQualifiers::Const;
				}
			}
		}

		return parsed;
	}

	LuaStateManager state_;
	ScriptTableMap tableMap_;
};