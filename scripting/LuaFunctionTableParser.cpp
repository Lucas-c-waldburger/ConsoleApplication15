#include "LuaFunctionTableParser.h"
#include "LuaStateManager.h"
#include "../deps/ctre/ctre.hpp"

template <typename T>
Result<T>
LuaFunctionTableParser::ParseLuaFunctionTableImpl(const sol::table& fnTable, const LuaStateManager& state)
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

	T parsed;

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

		sol::table fnSig = signatures[fnNameStr];
		if (!fnSig.valid())
		{
			return MAKE_ERROR_FMT("Table function '{}' did not have a matching metatable signature",
				fnNameStr);
		}

		auto [it, fnNameInserted] = parsed.try_emplace(fnNameStr, typename T::mapped_type{});
		assert(fnNameInserted);

		auto& valueVec = it->second;
		valueVec.reserve(fnSig.size());

		for (auto&& [_, arg] : fnSig)
		{
			if (arg.get_type() != sol::type::string)
			{
				return MAKE_ERROR("Argument name was not of type string");
			}

			auto argStr = arg.as<std::string>();

			if constexpr (std::same_as<T, ParsedLuaFunctionTableSignatures>)
			{
				TRY_ASSIGN(valueVec.emplace_back(), ParseFullArgumentType(argStr, state));
			}
			else
			{
				valueVec.emplace_back(std::move(argStr));
			}
		}
	}

	return parsed;
}

template Result<ParsedLuaFunctionTableSignatures>
LuaFunctionTableParser::ParseLuaFunctionTableImpl(const sol::table& fnTable, const LuaStateManager& state);

template Result<ParsedLuaFunctionTableStrings>
LuaFunctionTableParser::ParseLuaFunctionTableImpl(const sol::table& fnTable, const LuaStateManager& state);

Result<ParsedLuaFunctionTableSignatures> 
LuaFunctionTableParser::ParseLuaFunctionTableSignatures(const sol::table& fnTable, const LuaStateManager& state)
{
	return ParseLuaFunctionTableImpl<ParsedLuaFunctionTableSignatures>(fnTable, state);
}

Result<ParsedLuaFunctionTableStrings> 
LuaFunctionTableParser::ParseLuaFunctionTableStrings(const sol::table& fnTable, const LuaStateManager& state)
{
	return ParseLuaFunctionTableImpl<ParsedLuaFunctionTableStrings>(fnTable, state);
}

Result<uint64_t> 
LuaFunctionTableParser::ParseFullArgumentType(const std::string& argStr, const LuaStateManager& state)
{
	static constexpr auto regex = ctll::fixed_string{
		R"(^\s*(?:(?<leading_const>const)\s+)?(?<type>[A-Za-z_][A-Za-z0-9_]*)(?:\s+(?<trailing_const>const))?\s*(?<qualifier>[&*])?\s*$)"
	};

	auto match = ctre::match<regex>(argStr);
	if (!match)
	{
		return MAKE_ERROR_FMT("Argument string '{}' did not match regex", argStr);
	}

	uint32_t typeId = kInvalidLuaTypeId;
	uint32_t qualifiers = 0;

	auto typeName = match.get<"type">().to_view();
	if (typeName.empty())
	{
		return MAKE_ERROR_FMT("Argument string '{}' missing type name", argStr);
	}

	if (!state.IsRegistered(typeName))
	{
		return MAKE_ERROR_FMT("Argument type '{}' not registered with lua state", typeName);
	}

	typeId = state.GetRegisteredTypeId(typeName);
	assert(typeId != kInvalidLuaTypeId);

	const auto qual = match.get<"qualifier">().to_view();

	qualifiers |= (qual == "&" ? LuaTypeQualifiers::Ref :
				   qual == "*" ? LuaTypeQualifiers::Ptr : 0);

	const bool isRefOrPtr = (qualifiers & (LuaTypeQualifiers::Ref | LuaTypeQualifiers::Ptr)) != 0;
	if (isRefOrPtr)
	{
		if (IsNativeLuaType(typeName))
		{
			LOG_WARNING_FMT("Native lua type argument '{}' was marked as a reference or pointer, "
				"but native types can only be passed by value. Consider wrapping the argument in "
				"a user type to get reference behavior", typeName);

			qualifiers = 0;
		}
		else
		{
			// only add const if type is not value-only
			const bool isConst = !match.get<"leading_const">().to_view().empty() ||
								 !match.get<"trailing_const">().to_view().empty();
			if (isConst)
			{
				qualifiers |= LuaTypeQualifiers::Const;
			}
		}
	}

	return uint64_t{
		(static_cast<uint64_t>(qualifiers) << 32) | static_cast<uint64_t>(typeId)
	};
}
