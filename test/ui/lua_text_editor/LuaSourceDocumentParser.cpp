#include "LuaSourceDocumentParser.h"

#if IMGUI_ENABLED
#include <cassert>
#include "../../core/Result.h"

namespace ui {

namespace {

struct NodeName
{
    static constexpr std::string_view kFunctionCall = "function_call";
    static constexpr std::string_view kAssignmentStatement = "assignment_statement";
    static constexpr std::string_view kVariableDeclaration = "variable_declaration";
    static constexpr std::string_view kVariableList = "variable_list";
    static constexpr std::string_view kExpressionList = "expression_list";
    static constexpr std::string_view kTableConstructor = "table_constructor";
    static constexpr std::string_view kFunctionDeclaration = "function_declaration";
    static constexpr std::string_view kDotIndexExpression = "dot_index_expression";
    static constexpr std::string_view kMethodIndexExpression = "method_index_expression";
    static constexpr std::string_view kIdentifier = "identifier";
    static constexpr std::string_view kArguments = "arguments";
    static constexpr std::string_view kParameters = "parameters";
    static constexpr std::string_view kField = "field";
    static constexpr std::string_view kString = "string";
    static constexpr std::string_view kStringContent = "string_content";
    static constexpr std::string_view kBlock = "block";
};

static constexpr std::string_view kSetMetatableName = "setmetatable";
static constexpr std::string_view kSignaturesName = "__signatures";

constexpr bool kContinueParsing = true;
constexpr bool kAbandonParsing = false;

using FunctionNameToBlockIndexMap = std::unordered_map<std::string_view, size_t>;

bool IsMetatableNode(const TreeNode& node)
{
    return node.GetType() == NodeName::kIdentifier && node.GetText() == kSetMetatableName;
}

bool IsSignaturesNode(const TreeNode& node)
{
	return node.GetType() == NodeName::kIdentifier && node.GetText() == kSignaturesName;
}

struct GoToNodeArgs
{
    std::string_view name;
    int childCount = -1;
};

struct IndexExpressionText
{
	std::string_view lhs;
	std::string_view rhs;

	bool Empty() const noexcept { return lhs.empty() || rhs.empty(); }
};

IndexExpressionText GetIndexExpressionText(const TreeNode& node)
{
	assert(node.GetType() == NodeName::kDotIndexExpression || 
		   node.GetType() == NodeName::kMethodIndexExpression);

	IndexExpressionText result;

	if (node.GetNamedChildCount() != 2)
	{
		return result;
	}

	auto lhsNode = node.GetNamedChild(0);
	auto rhsNode = node.GetNamedChild(1);
	if (lhsNode.GetType() != NodeName::kIdentifier || rhsNode.GetType() != NodeName::kIdentifier)
	{
		return result;
	}

	result.lhs = lhsNode.GetText();
	result.rhs = rhsNode.GetText();

	return result;
}

std::vector<std::string_view> GetParameterText(TreeCursor& cursor)
{
	assert(cursor.CurrentNode().GetType() == NodeName::kParameters);

	if (!cursor.GoToFirstNamedChild())
	{
		return {};
	}

	std::vector<std::string_view> result;
	do
	{
		auto node = cursor.CurrentNode();
		if (node.GetType() != NodeName::kIdentifier)
		{
			continue;
		}

		auto paramName = node.GetText();
		if (!paramName.empty())
		{
			result.emplace_back(paramName);
		}

	} while (cursor.GoToNextNamedSibling());

	return result;
}

bool GoToFirstChildIfCurrentNodeName(TreeCursor& cursor, std::string_view name)
{
    auto node = cursor.CurrentNode();

    return node.GetType() == name && cursor.GoToFirstNamedChild();
}

bool GoToFirstChildIfChildNodeName(TreeCursor& cursor, std::string_view name)
{
	if (!cursor.GoToFirstNamedChild())
	{
		return false;
	}

	if (cursor.CurrentNode().GetType() != name)
	{
		cursor.GoToParent();

		return false;
	}

	return true;
}

//template <typename Fn> requires std::is_invocable_r_v<bool, Fn, const TreeNode&>
bool GoToSiblingWithNodeNameAndText(TreeCursor& cursor, std::string_view name, std::string_view text)
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

std::string_view GetFieldNodeStringContent(TreeCursor& cursor, LuaSourceDocument& doc)
{
	assert(cursor.CurrentNode().GetType() == NodeName::kField);

	if (!GoToFirstChildIfChildNodeName(cursor, NodeName::kString))
	{
		doc.AddError(cursor.CurrentNode(), "Expected named child 'string' in field node");
		return {};
	}

	if (!GoToFirstChildIfChildNodeName(cursor, NodeName::kStringContent))
	{
		doc.AddError(cursor.CurrentNode(), "Expected named child 'string_content' in string node");
		return {};
	}

	const auto text = cursor.CurrentNode().GetText();
	if (text.empty())
	{
		doc.AddError(cursor.CurrentNode(), "'string_content' text in string node was empty");
		return {};
	}

	return text;
}

bool ParseTableInfo(TreeCursor& cursor, LuaSourceDocument& doc)
{
    if (!cursor.GoToFirstChild())
    {
        doc.AddError(cursor.CurrentNode(), "Failed to go to first child of root cursor");

        return kAbandonParsing;
    }

    do
    {
        if (!GoToFirstChildIfCurrentNodeName(cursor, NodeName::kFunctionCall))
        {
            continue;
        }

        do
        {
            if (!IsMetatableNode(cursor.CurrentNode()))
            {
                continue;
            }

            while (cursor.GoToNextSibling())
            {
                auto node = cursor.CurrentNode();

                if (node.GetType() != NodeName::kArguments)
                {
                    continue;
                }

                if (node.GetNamedChildCount() != 2)
                {
                    doc.AddError(node, "Expected 2 named children in metatable args node");

                    return kAbandonParsing;
                }

                if (!cursor.GoToFirstNamedChild())
                {
                    doc.AddError(node, "Failed to go to first named child of metatable args node");

                    return kAbandonParsing;
                }

                node = cursor.CurrentNode();
                if (node.GetType() != NodeName::kIdentifier)
                {
                    doc.AddError(node, "Expected first named child of metatable args node to "
                                "be an identifier");

                    return kAbandonParsing;
                }

                doc.tableInfo.functionTableName = node.GetText();

                if (!cursor.GoToNextNamedSibling())
                {
                    doc.AddError(node, "Failed to go to second named child of metatable args node");

                    return kAbandonParsing;
                }

                node = cursor.CurrentNode();
                if (node.GetType() != NodeName::kIdentifier)
                {
                    doc.AddError(node, "Expected second named child of metatable args node "
                                 "to be an identifier");

                    return kAbandonParsing;
                }

                doc.tableInfo.metaTableName = node.GetText();

                return kContinueParsing;
            }

        } while (cursor.GoToNextSibling());

        cursor.GoToParent();

    } while (cursor.GoToNextSibling());

    doc.AddError(cursor.CurrentNode(), "Failed to parse metatable function call and arguments");

    return kAbandonParsing;
}

bool ParseSignaturesTableImpl(TreeCursor& cursor, LuaSourceDocument& doc,
							  FunctionNameToBlockIndexMap& fnNameToBlockIdxMap)
{
	auto node = cursor.CurrentNode();
	assert(node.GetType() == NodeName::kTableConstructor);

	if (!cursor.GoToFirstChild())
	{
		doc.AddError(node, "Failed to go to first child of signatures table constructor node");

		return kAbandonParsing;
	}

	cursor.SetHomeDepth();

	do
	{
		node = cursor.CurrentNode();
		if (node.GetType() != NodeName::kField)
		{
			continue;
		}

		if (!GoToFirstChildIfChildNodeName(cursor, NodeName::kIdentifier))
		{
			doc.AddError(node, "Expected first child of signature field node to be identifier");
			continue;
		}

		node = cursor.CurrentNode();

		auto fnIdent = node.GetText();
		if (fnIdent.empty())
		{
			doc.AddError(node, "Signature field identifier text was empty");
			cursor.GoToHomeDepth();

			continue;
		}

		if (fnNameToBlockIdxMap.contains(fnIdent))
		{
			doc.AddError(node, std::format(
				"Duplicate function name in signatures table: '{}'", fnIdent));
			cursor.GoToHomeDepth();

			continue;
		}

		if (!GoToSiblingWithNodeName(cursor, NodeName::kTableConstructor))
		{
			doc.AddError(cursor.CurrentNode(), "Expected second node in signatures table "
						 "field to be table_constructor (parameter types)");
			cursor.GoToHomeDepth();

			continue;
		}

		fnNameToBlockIdxMap[fnIdent] = doc.functionBlocks.size();
		
		auto& fnBlockInfo = doc.functionBlocks.emplace_back();
		fnBlockInfo.functionName = fnIdent;

		auto paramCursor = cursor.Branch();
		cursor.GoToHomeDepth();

		if (!paramCursor.GoToFirstNamedChild())
		{
			// assume no parameters for function
			continue;
		}

		do
		{
			if (paramCursor.CurrentNode().GetType() != NodeName::kField)
			{
				continue;
			}

			auto paramType = GetFieldNodeStringContent(paramCursor, doc);
			if (!paramType.empty())
			{
				fnBlockInfo.parameterTypes.emplace_back(paramType);
			}

			paramCursor.GoToHomeDepth();

		} while (paramCursor.GoToNextNamedSibling());

	} while (cursor.GoToNextNamedSibling());

	return kContinueParsing;
}

bool ParseSignaturesTable(TreeCursor& cursor, LuaSourceDocument& doc, 
                          FunctionNameToBlockIndexMap& fnNameToBlockIdxMap)
{
	if (!cursor.GoToFirstChild())
	{
		doc.AddError(cursor.CurrentNode(), "Failed to go to first child of root cursor");

		return kAbandonParsing;
	}

	cursor.SetHomeDepth();

	do // loop through all root children
	{
		if (!GoToFirstChildIfCurrentNodeName(cursor, NodeName::kVariableDeclaration))
		{
			continue;
		}

		// Find 'assignment_statement' child of this variable_declaration
		if (!GoToSiblingWithNodeName(cursor, NodeName::kAssignmentStatement))
		{
			cursor.GoToHomeDepth(); // back to root child loop
			continue;
		}

		// Now cursor is on assignment_statement. Check if its variable_list contains our metatable name.
		if (!cursor.GoToFirstNamedChild())
		{
			cursor.GoToHomeDepth(); // back to root child loop
			continue;
		}

		// Search for variable_list
		if (!GoToSiblingWithNodeName(cursor, NodeName::kVariableList))
		{
			cursor.GoToHomeDepth(); // back to root child loop
			continue;
		}

		// Now cursor is on variable_list. Search for identifier matching metaTableName.
		if (!cursor.GoToFirstNamedChild())
		{
			cursor.GoToHomeDepth(); // back to root child loop
			continue;
		}

		assert(!doc.tableInfo.metaTableName.empty());

		if (!GoToSiblingWithNodeNameAndText(cursor, NodeName::kIdentifier, 
											doc.tableInfo.metaTableName))
		{
			cursor.GoToHomeDepth(); // back to root child loop
			continue;
		}

		// Found the metatable identifier. Now go back to assignment_statement and find expression_list.
		cursor.GoToParent(2); // identifier -> variable_list -> assignment_statement

		if (!cursor.GoToFirstNamedChild())
		{
			cursor.GoToHomeDepth(); // back to root child loop
			continue;
		}
		 
		// Find expression_list sibling in assignment_statement
		if (!GoToSiblingWithNodeName(cursor, NodeName::kExpressionList))
		{
			cursor.GoToHomeDepth(); // back to root child loop
			continue;
		}

		// Cursor is now on expression_list. Search for table_constructor.
		if (!cursor.GoToFirstNamedChild())
		{
			cursor.GoToHomeDepth(); // back to root child loop
			continue;
		}

		if (!GoToSiblingWithNodeName(cursor, NodeName::kTableConstructor))
		{
			cursor.GoToHomeDepth(); // back to root child loop
			continue;
		}

		// Cursor is on table_constructor. Search for 'field' children.
		if (!cursor.GoToFirstNamedChild())
		{
			cursor.GoToHomeDepth(); // back to root child loop
			continue;
		}

		do
		{
			if (cursor.CurrentNode().GetType() != NodeName::kField)
			{
				continue;
			}

			if (!cursor.GoToFirstNamedChild())
			{
				continue;
			}

			if (!IsSignaturesNode(cursor.CurrentNode()))
			{
				cursor.GoToParent(); // back to field 
				continue;
			}
			// next sib should be table_constructor
			if (!GoToSiblingWithNodeName(cursor, NodeName::kTableConstructor)) 
			{
				cursor.GoToParent(); // back to field
				continue;
			}

			auto signaturesCursor = cursor.Branch();
			cursor.GoToParent();

			// get table function names + param types
			return ParseSignaturesTableImpl(signaturesCursor, doc, fnNameToBlockIdxMap);

		} while (cursor.GoToNextNamedSibling());

		// If we reach here, we didn't find __signatures in this variable_declaration.
		// Go back to root list and continue with the next one.
		cursor.GoToHomeDepth(); 

	} while (cursor.GoToNextSibling());

	doc.AddError(cursor.CurrentNode(), "Could not parse signatures table");

	return kAbandonParsing;
}

bool ParseTableFunctionBlocks(TreeCursor& cursor, LuaSourceDocument& doc,
							  const FunctionNameToBlockIndexMap& fnNameToBlockIdxMap)
{
	if (!cursor.GoToFirstChild())
	{
		doc.AddError(cursor.CurrentNode(), "Failed to go to first child of root cursor");
		return kAbandonParsing;
	}

	cursor.SetHomeDepth();

	do // loop through all root children
	{
		auto node = cursor.CurrentNode();
		if (node.GetType() != NodeName::kFunctionDeclaration)
		{
			continue;
		}

		SourceLocation functionDeclLocation = node.GetSourceLocation();

		if (!cursor.GoToFirstNamedChild())
		{
			doc.AddError(node, "Failed to go to first named child of function_declaration node");
			continue;
		}

		if (!GoToSiblingWithNodeName(cursor, NodeName::kDotIndexExpression))
		{
			cursor.GoToHomeDepth();
			continue;
		}

		auto indexExpressionText = GetIndexExpressionText(cursor.CurrentNode());
		if (indexExpressionText.Empty())
		{
			doc.AddError(cursor.CurrentNode(), "Failed to get index expression text");

			cursor.GoToHomeDepth();
			continue;
		}

		if (indexExpressionText.lhs != doc.tableInfo.functionTableName)
		{
			// This function declaration is not part of the function table we're interested in.
			cursor.GoToHomeDepth();
			continue;
		}

		auto it = fnNameToBlockIdxMap.find(indexExpressionText.rhs);
		if (it == fnNameToBlockIdxMap.end())
		{
			doc.AddError(cursor.CurrentNode(), std::format(
				"Function '{}' not found in function blocks map", 
				indexExpressionText.rhs));

			cursor.GoToHomeDepth();
			continue;
		}

		assert(it->second < doc.functionBlocks.size());
		auto& fnBlock = doc.functionBlocks[it->second];

		if (!GoToSiblingWithNodeName(cursor, NodeName::kParameters))
		{
			cursor.GoToHomeDepth();
			continue;
		}

		fnBlock.parameterNames = GetParameterText(cursor);
		if (fnBlock.parameterNames.size() != fnBlock.parameterTypes.size())
		{
			doc.AddError(cursor.CurrentNode(), std::format(
				"Parameter count mismatch for function '{}': "
				"expected {}, got {}", fnBlock.functionName, fnBlock.parameterTypes.size(), 
				fnBlock.parameterNames.size()));

			cursor.GoToHomeDepth();
			continue;
		}

		fnBlock.location = functionDeclLocation;

		cursor.GoToHomeDepth();

	} while (cursor.GoToNextNamedSibling());

	return kContinueParsing;
}

void ValidateParsedDocument(LuaSourceDocument& doc,
	const FunctionNameToBlockIndexMap& fnNameToBlockIdxMap)
{
	bool valid = true;
	if (doc.tableInfo.functionTableName.empty())
	{
		LOG_WARNING("Parsed document has empty function table name");
		valid = false;
	}
	if (doc.tableInfo.metaTableName.empty())
	{
		LOG_WARNING("Parsed document has empty metatable name");
		valid = false;
	}
	if (doc.functionBlocks.size() != fnNameToBlockIdxMap.size())
	{
		LOG_WARNING("Parsed document has function block count mismatch: "
			"{} blocks, {} entries in function name to block index map",
			doc.functionBlocks.size(), fnNameToBlockIdxMap.size());
		valid = false;
	}
	for (const auto& fnBlock : doc.functionBlocks)
	{
		if (fnBlock.functionName.empty())
		{
			LOG_WARNING("Parsed document has function block with empty function name");
			valid = false;
		}
		if (fnBlock.parameterNames.size() != fnBlock.parameterTypes.size())
		{
			LOG_WARNING("Parsed document has function block '{}' with parameter count mismatch: "
				"{} names, {} types", fnBlock.functionName, fnBlock.parameterNames.size(),
				fnBlock.parameterTypes.size());
			valid = false;
		}
	}

	if (!valid)
	{
		doc = {};
	}
}

} // unnamed


LuaSourceDocument ui::LuaSourceDocument::Parse(TreeCursor& rootCursor)
{
    LuaSourceDocument doc{};

    if (!ParseTableInfo(rootCursor, doc))
    {
		return {};
    }
	rootCursor.Reset();

    FunctionNameToBlockIndexMap fnNameToBlockIdxMap;

	if (!ParseSignaturesTable(rootCursor, doc, fnNameToBlockIdxMap))
	{
		return {};
	}
	rootCursor.Reset();

	if (!ParseTableFunctionBlocks(rootCursor, doc, fnNameToBlockIdxMap))
	{
		return {};
	}

	ValidateParsedDocument(doc, fnNameToBlockIdxMap);

	return doc;
}

void LuaSourceDocument::AddError(const TreeNode& node, std::string_view msg)
{
	errors.emplace_back(std::string{ msg }, node.GetSourceLocation());
}

} // ui

#endif