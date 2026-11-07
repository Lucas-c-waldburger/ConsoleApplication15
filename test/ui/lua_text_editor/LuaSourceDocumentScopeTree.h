#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "LuaSourceParserCommon.h"
#include <vector>

namespace ui {

struct TreeCursor;

namespace lua {

struct LuaSourceDocumentScope
{
	size_t index = std::numeric_limits<size_t>::max();
	size_t parent = std::numeric_limits<size_t>::max();
	std::vector<size_t> children;

	bool IsLeaf() const { return children.empty(); }
};

struct LuaSourceDocumentScopeTree
{
	size_t current = 0;

	void Reset() { current = 0; }
	bool GoToParent();

	bool PushScope(TreeCursor& cursor);
	bool AtRoot() const noexcept { return current <= 0; }
	bool AtLeaf() const;
	bool CurrentValid() const;
	bool SetCurrent(size_t idx);

	size_t Size() const noexcept;

	SourceLocation GetCurrentScopeLocation() const;
	std::string_view GetCurrentScopeName() const;
	size_t GetDocumentEnd() const;

	bool InsideFunctionScope();
	size_t GetParentFunctionScopeIndex();

	std::vector<SourceLocation> locations;
	std::vector<std::string_view> names;
	std::vector<uint32_t> returns;
	std::vector<LuaSourceDocumentScope> nodes;
};

} // lua
} // ui

#endif