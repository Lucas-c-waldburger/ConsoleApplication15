#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "TreeCursor.h"
#include <unordered_map>

namespace ui {

struct LuaSourceDocument
{
	struct TableInfo
	{
		std::string_view functionTableName;
		std::string_view metaTableName;
	};

	struct FunctionBlockInfo
	{
		SourceLocation location;
		std::string_view functionName;
		std::vector<std::string_view> parameterNames;
		std::vector<std::string_view> parameterTypes;
	};

	TableInfo tableInfo;
	std::vector<FunctionBlockInfo> functionBlocks;
	std::vector<SourceDocumentError> errors;

	static LuaSourceDocument Parse(TreeCursor& rootCursor);

	void AddError(const TreeNode& node, std::string_view msg);
};


} // ui

#endif
