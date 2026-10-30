#include "LuaSourceParserCommon.h"

#if IMGUI_ENABLED
#include <cassert>
#include "TreeNode.h"

namespace ui {

bool IsIndexExpressionNode(const TreeNode& node)
{
	return node.GetType() == NodeName::kDotIndexExpression ||
		   node.GetType() == NodeName::kMethodIndexExpression;
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

} // ui

#endif