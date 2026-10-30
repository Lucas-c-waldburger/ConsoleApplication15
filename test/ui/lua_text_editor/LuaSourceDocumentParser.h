#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "TreeCursor.h"
#include <unordered_map>

class ParsedLuaUserTypeInfo;

namespace ui {

template <typename T>
struct FunctionBlockInfoTemplate
{
	SourceLocation location;
	std::string_view functionName;
	std::vector<T> parameterNames;
	std::vector<T> parameterTypes;
};

extern template struct FunctionBlockInfoTemplate<std::string_view>;
extern template struct FunctionBlockInfoTemplate<std::string>;

struct LuaSourceDocument
{
	struct SignaturesTableInfo
	{
		std::vector<std::string_view> functionNames;
		std::vector<std::vector<std::string_view>> parameterTypes;
	};

	struct TableInfo
	{
		std::string_view functionTableName;
		std::string_view metaTableName;
		SignaturesTableInfo signatures;
	};

	enum class IndexExpressionType
	{
		Dot,
		Method
	};

	struct IndexExpressionInfo
	{
		SourceLocation location;
		std::string_view lhsText;
		std::string_view rhsText;
		IndexExpressionType type = IndexExpressionType::Dot;
	};

	enum class AssignmentStatementRhsType
	{
		Unknown,
		NumberLiteral,
		StringLiteral,
		FunctionReturn
	};

	struct AssignmentStatementInfo
	{
		SourceLocation location;
		std::string_view lhsText;
		std::string_view rhsText;
		AssignmentStatementRhsType rhsType = AssignmentStatementRhsType::Unknown;
	};

	using FunctionBlockInfo = FunctionBlockInfoTemplate<std::string_view>;

	TableInfo tableInfo;
	std::vector<FunctionBlockInfo> functionBlocks;
	std::vector<IndexExpressionInfo> indexExpressions;
	std::vector<SourceDocumentError> errors;

	static LuaSourceDocument Parse(TreeCursor& rootCursor);
	static LuaSourceDocument Parse(TreeCursor& rootCursor, const ParsedLuaUserTypeInfo& userTypeInfo);

	void AddError(const TreeNode& node, std::string_view msg);
};


} // ui

#endif
