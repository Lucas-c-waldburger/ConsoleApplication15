#include "LuaSourceParserCommon.h"

#if IMGUI_ENABLED
#include <cassert>
#include "TreeCursor.h"

namespace ui {

bool IsIndexExpressionNode(const TreeNode& node)
{
	return node.GetType() == NodeName::kDotIndexExpression ||
		   node.GetType() == NodeName::kMethodIndexExpression;
}
bool IsMetatableNode(const TreeNode& node)
{
	return node.GetType() == NodeName::kIdentifier && 
		   node.GetText() == kSetMetatableName;
}
bool IsSignaturesNode(const TreeNode& node)
{
	return node.GetType() == NodeName::kIdentifier && 
		   node.GetText() == kSignaturesName;
}

bool IsListNode(const TreeNode& node)
{
	return node.GetType() == NodeName::kVariableList ||
		   node.GetType() == NodeName::kExpressionList;
}

std::optional<IndexExpressionInfo>
ExtractIndexExpressionInfo(const TreeNode& node)
{
	assert(IsIndexExpressionNode(node));

	IndexExpressionInfo result;

	result.location = node.GetSourceLocation();
	result.type = (node.GetType() == NodeName::kDotIndexExpression)
		? IndexExpressionType::Dot
		: IndexExpressionType::Method;

	if (node.GetNamedChildCount() != 2)
	{
		return std::nullopt;
	}

	auto lhsNode = node.GetNamedChild(0);
	auto rhsNode = node.GetNamedChild(1);

	if (lhsNode.GetType() != NodeName::kIdentifier ||
		rhsNode.GetType() != NodeName::kIdentifier)
	{
		return std::nullopt;
	}

	if (result.lhsText = lhsNode.GetText(); result.lhsText.empty())
	{
		return std::nullopt;
	}
	if (result.rhsText = rhsNode.GetText(); result.rhsText.empty())
	{
		return std::nullopt;
	}

	return result;
}

bool GoToSiblingWithNodeName(TreeCursor& cursor, std::string_view name)
{
	do
	{
		if (cursor.CurrentNode().GetType() == name)
		{
			return true;
		}
	} while (cursor.GoToNextNamedSibling());

	return false;
}

bool GoToSiblingWithNodeNameAndText(TreeCursor& cursor, std::string_view name, 
									std::string_view text)
{
	do
	{
		auto node = cursor.CurrentNode();
		if (node.GetType() == name && node.GetText() == text)
		{
			return true;
		}
	} while (cursor.GoToNextNamedSibling());

	return false;
}

bool GoToSiblingWithNodeNameAndNamedChildCount(
	TreeCursor& cursor, std::string_view name, size_t count)
{
	do
	{
		auto node = cursor.CurrentNode();
		if (node.GetType() == name && node.GetNamedChildCount() == count)
		{
			return true;
		}
	} while (cursor.GoToNextNamedSibling());

	return false;
}

} // ui

#endif