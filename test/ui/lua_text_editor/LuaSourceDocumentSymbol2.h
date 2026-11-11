#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "LuaSourceParserCommon.h"
#include "../../core/StableSOA.h"

namespace ui::lua {

struct LuaSourceDocumentSymbol
{
	enum class Kind : uint8_t
	{
		Invalid,
		Local,
		ScopedGlobal,
		UnscopedGlobal
	};

	std::string_view name;
	SourceLocation location;
	SourceLocation scope;
	uint32_t type = std::numeric_limits<uint32_t>::max();
	Kind kind = Kind::Invalid;

	constexpr bool operator==(const LuaSourceDocumentSymbol&) const = default;
};

using LuaSourceDocumentSymbolSOA = StableSOA<
	LuaSourceDocumentSymbol,
	&LuaSourceDocumentSymbol::name,
	&LuaSourceDocumentSymbol::location,
	&LuaSourceDocumentSymbol::scope,
	&LuaSourceDocumentSymbol::type,
	&LuaSourceDocumentSymbol::kind
>;

} // ui::lua

#endif