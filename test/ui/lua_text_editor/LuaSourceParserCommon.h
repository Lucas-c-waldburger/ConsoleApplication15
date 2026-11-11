#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include <string>
#include <string_view>
#include <optional>
#include "../../scripting/ParsedLuaUserTypeInfo.h"

namespace ui {

struct TreeNode;
struct TreeCursor;

static constexpr std::string_view kChunkScopeName = "__chunk";
static constexpr std::string_view kBlockScopeName = "__block";

struct SourceLocation
{
	size_t start = 0;
	size_t end = 0;

	constexpr bool operator<(const SourceLocation& rhs) const noexcept
	{
		return start < rhs.start;
	}
	constexpr bool operator>(const SourceLocation& rhs) const noexcept
	{
		return start > rhs.start;
	}
	constexpr bool operator==(const SourceLocation& rhs) const noexcept
	{
		return start == rhs.start && end == rhs.end;
	}
	constexpr bool Overlaps(const SourceLocation& rhs) const noexcept
	{
		return start < rhs.end && rhs.start < end;
	}
	constexpr bool Inside(const SourceLocation& rhs) const noexcept
	{
		return start >= rhs.start && end <= rhs.end;
	}
	constexpr bool Contains(size_t byteOffset) const noexcept
	{
		return start <= byteOffset && byteOffset <= end;
	}
};

struct SourceDocumentError
{
	enum class Type
	{
		TableFunctionNotFoundInSignatures,
		SignatureFunctionNotFoundForTable,
		ParameterCountMismatch,
		UnrecognizedMember
	};

	bool operator==(const SourceDocumentError& rhs) const = default;

	std::string message;
	SourceLocation location;
};

struct NodeName
{
	static constexpr std::string_view kFunctionCall = "function_call";
	static constexpr std::string_view kAssignmentStatement = "assignment_statement";
	static constexpr std::string_view kVariableDeclaration = "variable_declaration";
	static constexpr std::string_view kVariableList = "variable_list";
	static constexpr std::string_view kExpressionList = "expression_list";
	static constexpr std::string_view kTableConstructor = "table_constructor";
	static constexpr std::string_view kFunctionDeclaration = "function_declaration";
	static constexpr std::string_view kFunctionDefinition = "function_definition";
	static constexpr std::string_view kDotIndexExpression = "dot_index_expression";
	static constexpr std::string_view kMethodIndexExpression = "method_index_expression";
	static constexpr std::string_view kIdentifier = "identifier";
	static constexpr std::string_view kArguments = "arguments";
	static constexpr std::string_view kParameters = "parameters";
	static constexpr std::string_view kField = "field";
	static constexpr std::string_view kNumber = "number";
	static constexpr std::string_view kBoolean = "boolean";
	static constexpr std::string_view kString = "string";
	static constexpr std::string_view kStringContent = "string_content";
	static constexpr std::string_view kBlock = "block";
	static constexpr std::string_view kChunk = "chunk";
	static constexpr std::string_view kReturnStatement = "return_statement";
	static constexpr std::string_view kTypedIdentifier = "typed_identifier";
	static constexpr std::string_view kForStatement = "for_statement";
	static constexpr std::string_view kWhileStatement = "while_statement";
	static constexpr std::string_view kRepeatStatement = "repeat_statement";
	static constexpr std::string_view kDoStatement = "do_statement";
	static constexpr std::string_view kIfStatement = "if_statement";
};

static constexpr std::string_view kLocalName = "local";
static constexpr std::string_view kGlobalName = "global";
static constexpr std::string_view kFunctionName = "global";
static constexpr std::string_view kSetMetatableName = "setmetatable";
static constexpr std::string_view kSignaturesName = "__signatures";

bool IsIndexExpressionNode(const TreeNode& node);
bool IsMetatableNode(const TreeNode& node);
bool IsSignaturesNode(const TreeNode& node);
bool IsListNode(const TreeNode& node);

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

std::optional<IndexExpressionInfo>
ExtractIndexExpressionInfo(const TreeNode& node);

inline constexpr bool IsMemberKindField(LuaUsertypeMemberKind kind) noexcept
{
	return kind == LuaUsertypeMemberKind::Field || 
		   kind == LuaUsertypeMemberKind::Property;
}
inline constexpr bool IsMemberKindFunction(LuaUsertypeMemberKind kind) noexcept
{
	return kind == LuaUsertypeMemberKind::Method || 
		   kind == LuaUsertypeMemberKind::Overload;
}

bool GoToSiblingWithNodeName(TreeCursor& cursor, std::string_view name);

bool GoToSiblingWithNodeNameAndText(TreeCursor& cursor, std::string_view name, 
									std::string_view text);

bool GoToSiblingWithNodeNameAndNamedChildCount(TreeCursor& cursor, 
											   std::string_view name, size_t count);


} // ui

namespace std {

template <>
struct hash<ui::SourceLocation> {
	size_t operator()(ui::SourceLocation loc) const noexcept {
		return MakeHash(loc.start, loc.end);
	}
};

} // std

#endif