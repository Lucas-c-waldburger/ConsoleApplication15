#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED

namespace ui {

struct TreeCursor;
struct LuaSourceDocumentSymbolParseContext;

bool ParseTableInfo(TreeCursor& cursor,
	LuaSourceDocumentSymbolParseContext& ctx);

} // ui

#endif