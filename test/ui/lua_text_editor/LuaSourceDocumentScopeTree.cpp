#include "LuaSourceDocumentScopeTree.h"

#if IMGUI_ENABLED
#include "TreeCursor.h"
#include "../../scripting/LuaNativeTypeIdUtils.h"

namespace ui::lua {

namespace {

std::string_view GetScopeName(TreeCursor& cursor)
{
	if (cursor.CurrentNode().GetType() != NodeName::kFunctionDeclaration)
	{
		return kBlockScopeName;
	}

	if (!cursor.GoToFirstNamedChild())
	{
		return {};
	}

	std::string_view result;
	do
	{
		auto node = cursor.CurrentNode();

		if (node.GetType() == NodeName::kIdentifier ||
			node.GetType() == NodeName::kDotIndexExpression ||
			node.GetType() == NodeName::kMethodIndexExpression)
		{
			result = node.GetText();
			break;
		}

	} while (cursor.GoToNextNamedSibling());

	cursor.GoToParent();

	return result;
}

bool IsFunctionScope(std::string_view scopeName)
{
	return !scopeName.empty() && scopeName != kBlockScopeName;
}

} // unnamed

bool LuaSourceDocumentScopeTree::CurrentValid() const
{
	assert(locations.size() == names.size());
	assert(locations.size() == nodes.size());

	return current < nodes.size() && nodes[current].index == current;
}

bool LuaSourceDocumentScopeTree::SetCurrent(size_t idx)
{
	assert(locations.size() == names.size());
	assert(locations.size() == nodes.size());

	if (idx >= nodes.size())
	{
		return false;
	}

	current = idx;
	return true;
}

size_t LuaSourceDocumentScopeTree::Size() const noexcept
{
	assert(locations.size() == names.size());
	assert(locations.size() == nodes.size());

	return nodes.size();
}

bool LuaSourceDocumentScopeTree::GoToParent()
{
	assert(locations.size() == names.size());
	assert(locations.size() == nodes.size());

	if (current < nodes.size() && nodes[current].parent < nodes.size())
	{
		current = nodes[current].parent;
		return true;
	}
	return false;
}

bool LuaSourceDocumentScopeTree::PushScope(TreeCursor& cursor)
{
	assert(locations.size() == names.size());
	assert(locations.size() == nodes.size());

	const bool isRoot = nodes.empty();
	if (!isRoot && current >= nodes.size())
	{
		return false;
	}

	auto treeNode = cursor.CurrentNode();
	assert(treeNode);

	const size_t newIdx = nodes.size();

	if (!isRoot)
	{
		nodes[current].children.emplace_back(newIdx);
	}
	locations.emplace_back(treeNode.GetSourceLocation());
	names.emplace_back(GetScopeName(cursor));
	returns.emplace_back(kInvalidLuaTypeId);

	auto& newScope = nodes.emplace_back();
	newScope.index = newIdx;
	newScope.parent = isRoot
		? std::numeric_limits<size_t>::max()
		: current;

	current = newIdx;
	return true;
}

bool LuaSourceDocumentScopeTree::AtLeaf() const
{ 
	return current < nodes.size() && nodes[current].IsLeaf(); 
}

std::string_view LuaSourceDocumentScopeTree::GetCurrentScopeName() const
{
	if (!CurrentValid())
	{
		return {};
	}

	return names[current];
}

size_t LuaSourceDocumentScopeTree::GetDocumentEnd() const
{
	if (locations.empty())
	{
		return 0;
	}
	return locations[0].end;
}

bool LuaSourceDocumentScopeTree::InsideFunctionScope()
{
	if (!CurrentValid())
	{
		return false;
	}

	const size_t origScopeIdx = current;
	bool found = false;

	do
	{
		auto scopeName = GetCurrentScopeName();
		if (IsFunctionScope(scopeName))
		{
			found = true;
			break;
		}
	} while (GoToParent());

	SetCurrent(origScopeIdx);

	return found;
}

size_t LuaSourceDocumentScopeTree::GetParentFunctionScopeIndex()
{
	if (!CurrentValid())
	{
		return std::numeric_limits<size_t>::max();
	}

	const size_t origScopeIdx = current;
	size_t result = std::numeric_limits<size_t>::max();
	do
	{
		auto scopeName = GetCurrentScopeName();
		if (IsFunctionScope(scopeName))
		{
			result = current;
			break;
		}

	} while (GoToParent());

	SetCurrent(origScopeIdx);

	return result;
}

SourceLocation LuaSourceDocumentScopeTree::GetCurrentScopeLocation() const
{
	if (!CurrentValid())
	{
		return {};
	}

	return locations[current];
}



} // ui::lua

#endif