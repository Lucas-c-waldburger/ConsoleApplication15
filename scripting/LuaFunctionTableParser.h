#pragma once
#include <sol/sol.hpp>
#include "../core/Dictionary.h"
#include "../core/Result.h"

class LuaStateManager;

enum LuaTypeQualifiers : uint8_t
{
	Const = 1 << 0,
	Ref = 1 << 1,
	Ptr = 1 << 2
};

using ParsedLuaFunctionTableSignatures = UnorderedDictionary<std::vector<uint64_t>>;

class LuaFunctionTableParser
{
public:
	static Result<ParsedLuaFunctionTableSignatures>
	ParseLuaFunctionTableSignatures(sol::table& fnTable, const LuaStateManager& state);

	static Result<uint64_t> 
	ParseFullArgumentType(const std::string& argStr, const LuaStateManager& state);

private:
	LuaFunctionTableParser() = default;
};