#include "TreeNode.h"

#if IMGUI_ENABLED

namespace ui {

std::string_view TreeNode::GetText() const
{
    const uint32_t start = ts_node_start_byte(node);
    const uint32_t end = ts_node_end_byte(node);

    if (end >= start && end <= source.size())
    {
        return source.substr(start, end - start);
    }

    return {};
}

std::string_view TreeNode::GetType() const
{
    if (!ts_node_is_named(node))
    {
        return {};
    }

    return ts_node_type(node);
}

TreeNode TreeNode::FindChild(std::string_view name) const
{
    for (uint32_t i = 0; i < ts_node_child_count(node); ++i)
    {
        if (auto child = TreeNode(ts_node_child(node, i), source);
            child.GetType() == name)
        {
            return child;
        }
    }

    return {};
}

std::vector<TreeNode> TreeNode::GatherChildren(std::string_view name) const
{
	std::vector<TreeNode> children;
    for (uint32_t i = 0; i < ts_node_child_count(node); ++i)
    {
        if (auto child = TreeNode(ts_node_child(node, i), source);
            child.GetType() == name)
        {
            children.push_back(child);
		}
    }

	return children;
}

std::vector<TreeNode> TreeNode::GatherChildren(bool(*pred)(const TreeNode&)) const
{
    std::vector<TreeNode> children;
    for (uint32_t i = 0; i < ts_node_child_count(node); ++i)
    {
        if (auto child = TreeNode(ts_node_child(node, i), source);
            pred(child))
        {
            children.push_back(child);
        }
    }

    return children;
}

std::vector<TreeNode> TreeNode::GatherNamedChildren() const
{
    const size_t namedChildCount = ts_node_named_child_count(node);
    if (namedChildCount <= 0)
    {
        return {};
    }

    std::vector<TreeNode> children;
    children.reserve(namedChildCount);

    for (uint32_t i = 0; i < namedChildCount; ++i)
    {
        children.push_back(TreeNode(ts_node_child(node, i), source));
    }

    return children;
}

} // ui

#endif