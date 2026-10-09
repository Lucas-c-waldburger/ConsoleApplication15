#include "LuaSourceParser.h"

#if IMGUI_ENABLED
#include <cassert>
#include <unordered_map>
#include <ranges>
#include "TSEditorBridge.h"
#include "../../core/Logger.h"

namespace ui {

namespace {

// function_call -> setmetatable -> arguments (2) -> identifier: table , meta

// A) find meta: variable_declaration -> assignment_statement -> variable_list -> identifier[0] 
// B) find signatures: goto_parent (assignment_statement) -> expression_list -> table_constructor 
//  -> named field -> identifier[0] (__signatures)
// C) parse signatures: (same level as identifier), -> table_constructor
//  -> for each field child -> identifier[0] == function name 
//                          -> table_constructor -> for each field child 
//                              -> string -> string_content (param type)
// D) parse table function decls: back to setmetatable function_call -> argument[1] -> identifier: table
//  -> goto top level -> for each child 
//    -> if function_declaration -> dot_index_expression -> identifier[0] (named node) == table name
//                                                          identifier[2] (named node) == function name
//    -> skip if identifier[0] != table name
//    -> reference signatures table -> find matching function name && param
//      (validate param count matches arg count)
//    -> else goto parent -> find parameters -> for each named child identifier
//      -> associate index of named param node with index of __signature function arg 

static constexpr std::string_view kSetMetaTableName = "setmetatable";
static constexpr std::string_view kFunctionCallName = "function_call";
static constexpr std::string_view kAssignmentStatementName = "assignment_statement";
static constexpr std::string_view kVariableDeclarationName = "variable_declaration";
static constexpr std::string_view kVariableListName = "variable_list";
static constexpr std::string_view kExpressionListName = "expression_list";
static constexpr std::string_view kTableConstructorName = "table_constructor";
static constexpr std::string_view kFunctionDeclarationName = "function_declaration";
static constexpr std::string_view kDotIndexExpressionName = "dot_index_expression";
static constexpr std::string_view kIdentifierName = "identifier";
static constexpr std::string_view kArgumentsName = "arguments";
static constexpr std::string_view kParametersName = "parameters";
static constexpr std::string_view kFieldName = "field";
static constexpr std::string_view kStringName = "string";
static constexpr std::string_view kStringContentName = "string_content";

static constexpr bool kContinueParsing = true;
static constexpr bool kAbandonParsing = false;

SourceLocation GetNodeSourceLocation(const TreeNode& node)
{
    return SourceLocation{ node.GetStartPos(), node.GetEndPos() };
}

bool IsMetaTableNode(const TreeNode& node)
{
    return node.GetType() == kIdentifierName && node.GetText() == kSetMetaTableName;
}

bool IsSignaturesNode(const TreeNode& node)
{
    const auto type = node.GetType();
	const auto text = node.GetText();
    return node.GetType() == kIdentifierName && node.GetText() == "__signatures";
}

bool NodeMatchesMetaTableName(const TreeNode& node, std::string_view metaTableName)
{
    const auto type = node.GetType();
    const auto text = node.GetText();
    return node.GetType() == kIdentifierName && node.GetText() == metaTableName;
}

Result<std::string_view> GetFieldNodeStringContent(TreeCursor& cursor)
{
	auto node = cursor.CurrentNode();
    if (node.GetNamedChildCount() != 1)
    {
        return MAKE_ERROR("Expected 1 named child ('string') in field node");
	}

    if (!cursor.GoToFirstNamedChild())
    {
        return MAKE_ERROR("Failed to go to first named child of field node");
    }

    node = cursor.CurrentNode();
    if (node.GetType() != kStringName)
    {
        cursor.GoToParent();
        return MAKE_ERROR("Expected named child of field node to be a 'string' node");
    }

    if (node.GetNamedChildCount() != 1)
    {
        cursor.GoToParent();
        return MAKE_ERROR("Expected 1 named child ('string_content') in string node");
	}

	if (!cursor.GoToFirstNamedChild())
	{
		cursor.GoToParent();
		return MAKE_ERROR("Failed to go to first named child of string node");
	}

	node = cursor.CurrentNode();
	if (node.GetType() != kStringContentName)
	{
		cursor.GoToParent(2);
		return MAKE_ERROR("Expected named child of string node to be a 'string_content' node");
	}

	const auto text = node.GetText();
	if (text.empty())
	{
		cursor.GoToParent(2);
		return MAKE_ERROR("string_content node text was empty");
	}

	cursor.GoToParent(2);

	return text;
}

std::optional<size_t> 
GetDotIndexExpressionFunctionDataIndex(TreeCursor& cursor, const ParsedLuaSourceDocument& parsedDoc)
{
    auto node = cursor.CurrentNode();
    if (node.GetType() != kDotIndexExpressionName)
    {
        return std::nullopt;
    }

    if (node.GetNamedChildCount() != 2)
    {
        return std::nullopt;
    }

    if (!cursor.GoToFirstNamedChild())
    {
        return std::nullopt;
    }

    node = cursor.CurrentNode();
    if (node.GetType() != kIdentifierName)
    {
        return std::nullopt;
    }

    if (node.GetText() != parsedDoc.functionTableName)
    {
        return std::nullopt;
    }

    if (!cursor.GoToNextNamedSibling())
    {
        return std::nullopt;
    }

    node = cursor.CurrentNode();
    if (node.GetType() != kIdentifierName)
    {
        return std::nullopt;
    }

    const auto text = node.GetText();

    auto it = parsedDoc.functionDataIndex.find(text);

    return (it != parsedDoc.functionDataIndex.end()) ? it->second : std::optional<size_t>{};
}

Result<std::string_view> 
GetDotIndexExpressionFunctionName(TreeCursor& cursor, ParsedLuaSourceDocument& parsedDoc)
{
    auto node = cursor.CurrentNode();
    if (node.GetType() != kDotIndexExpressionName)
    {
        return MAKE_ERROR("Expected dot_index_expression node");
    }

    if (node.GetNamedChildCount() != 2)
    {
        return MAKE_ERROR("Expected 2 named children ('identifier', 'identifier') "
            "in dot index expression node");
    }

    if (!cursor.GoToFirstNamedChild())
    {
        return MAKE_ERROR("Failed to go to first named child of dot index expression node");
    }

    node = cursor.CurrentNode();
    if (node.GetType() != kIdentifierName)
    {
        cursor.GoToParent();

        return MAKE_ERROR("Expected named child 1 of dot index expression node to "
            "be an 'identifier' node");
    }

    if (node.GetText() != parsedDoc.functionTableName)
    {
        cursor.GoToParent();

        return std::string_view{};
    }

    if (!cursor.GoToNextNamedSibling())
    {
        cursor.GoToParent();

		return MAKE_ERROR("Failed to go to second named child of dot index "
            "expression node");
    }

    node = cursor.CurrentNode();
    if (node.GetType() != kIdentifierName)
    {
        cursor.GoToParent();

        return MAKE_ERROR("Expected named child 2 of dot index expression node to "
            "be an 'identifier' node");
    }

    const auto text = node.GetText();

    auto it = parsedDoc.functionDataIndex.find(text);
    if (it == parsedDoc.functionDataIndex.end())
    {
        parsedDoc.errors.emplace_back(
            std::format("Table function '{}' not found in parsed "
            "signatures table functions", text), GetNodeSourceLocation(node));

        cursor.GoToParent();

        return std::string_view{};
    }

    cursor.GoToParent();

    return text;
}

bool ParseSignatureTableFunctionAndArguments(TreeCursor& cursor, ParsedLuaSourceDocument& parsedDoc)
{
    auto node = cursor.CurrentNode();
    assert(node.GetType() == kTableConstructorName);

    if (!cursor.GoToFirstNamedChild())
    {
        parsedDoc.errors.emplace_back(
            "Failed to go to first named child of signatures table_constructor node",
            GetNodeSourceLocation(node));

        return kContinueParsing;
    }

    do
    {
        node = cursor.CurrentNode();
        if (node.GetType() != kFieldName)
        {
            continue;
        }

        if (node.GetNamedChildCount() != 2)
        {
            parsedDoc.errors.emplace_back(
                "Expected 2 named children in signatures field node "
                "(identifier and table_constructor)", GetNodeSourceLocation(node));

            continue;
		}

        if (!cursor.GoToFirstNamedChild())
        {
            parsedDoc.errors.emplace_back(
                "Failed to go to first named child of signatures field node",
                GetNodeSourceLocation(node));

			continue;
        }

		node = cursor.CurrentNode();
        if (node.GetType() != kIdentifierName)
        {
            parsedDoc.errors.emplace_back(
                "Expected first named child of signatures function "
                "field node to be an identifier", GetNodeSourceLocation(node));

            cursor.GoToParent();
            continue;
		}

        const size_t fnDataIdx = parsedDoc.functionArgumentTypes.size();

        const auto nodeText = node.GetText();
        if (nodeText.empty())
        {
            parsedDoc.errors.emplace_back(
                "signatures function identifier name was empty",
                GetNodeSourceLocation(node));

            cursor.GoToParent();
            continue;
		}

		auto [it, inserted] = parsedDoc.functionDataIndex.try_emplace(
            node.GetText(), fnDataIdx);
		if (!inserted)
		{
			parsedDoc.errors.emplace_back(
				std::format("Duplicate function name '{}' found in signatures table", 
					nodeText), GetNodeSourceLocation(node));

			cursor.GoToParent();
			continue;
		}

		parsedDoc.functionNames.emplace_back(nodeText);
        parsedDoc.functionParameterNames.emplace_back();
		parsedDoc.functionArgumentTypes.emplace_back();
        parsedDoc.functionBlockInfo.push_back({ .dataIndex = fnDataIdx });

        if (!cursor.GoToNextNamedSibling())
        {
            parsedDoc.errors.emplace_back(
                "Failed to go to second named child of signatures function "
                "field node", GetNodeSourceLocation(node));

            cursor.GoToParent();
			continue;
        }

        node = cursor.CurrentNode();
        if (node.GetType() != kTableConstructorName)
        {
            parsedDoc.errors.emplace_back(
                "Expected second named child of signatures function "
                "field node to be a table_constructor (arguments)", 
                GetNodeSourceLocation(node));

            cursor.GoToParent();
            continue;
        }

		if (!cursor.GoToFirstNamedChild())
		{
			parsedDoc.errors.emplace_back(
				"Failed to go to first named child of signatures "
				"function arguments table_constructor node",
				GetNodeSourceLocation(node));

			cursor.GoToParent(2);
			continue;
		}

		do // gather arguments
		{
			node = cursor.CurrentNode();
			if (node.GetType() != kFieldName)
			{
				continue;
			}

			auto argTypeResult = GetFieldNodeStringContent(cursor);
			if (!argTypeResult.Success())
			{
				parsedDoc.errors.emplace_back(
					argTypeResult.GetError().GetMessage(),
					GetNodeSourceLocation(node));

				continue;
			}

			parsedDoc.functionArgumentTypes.back().emplace_back(
				argTypeResult.GetValue());

		} while (cursor.GoToNextNamedSibling());

		cursor.GoToParent(2);

    } while (cursor.GoToNextNamedSibling());

    cursor.GoToParent();

    return kContinueParsing;
}

bool ParseMetaAndFunctionTableNames(TreeCursor& cursor, ParsedLuaSourceDocument& parsedDoc)
{
    if (!cursor.GoToFirstChild())
    {
		parsedDoc.errors.emplace_back("Failed to go to first child of root cursor");

        return kAbandonParsing;
    }

    do
    {
        auto node = cursor.CurrentNode();

        if (node.GetType() != kFunctionCallName)
        {
            continue;
        }

        if (!cursor.GoToFirstChild())
        {
            continue;
        }
            
        do
        {
            node = cursor.CurrentNode();

            if (!IsMetaTableNode(node))
            {
                continue;
            }
               
            while (cursor.GoToNextSibling())
            {
				node = cursor.CurrentNode();

                if (node.GetType() != kArgumentsName)
                {
                    continue;
                }

                if (node.GetNamedChildCount() != 2)
                {
                    parsedDoc.errors.emplace_back(
                        "Expected 2 named children in metatable arguments node",
                        GetNodeSourceLocation(node)
                    );

                    return kAbandonParsing;
                }
 
                if (!cursor.GoToFirstNamedChild())
                {
                    parsedDoc.errors.emplace_back(
                        "Failed to go to first named child of metatable arguments node",
                        GetNodeSourceLocation(node)
                    );

                    return kAbandonParsing;
				}

				node = cursor.CurrentNode();
				if (node.GetType() != kIdentifierName)
				{
					parsedDoc.errors.emplace_back(
						"Expected first named child of metatable arguments node to "
						"be an identifier", GetNodeSourceLocation(node));

					return kAbandonParsing;
				}

                parsedDoc.functionTableName = node.GetText();
                                        
				if (!cursor.GoToNextNamedSibling())
                {
                    parsedDoc.errors.emplace_back(
                        "Failed to go to second named child of metatable arguments node",
                        GetNodeSourceLocation(node)
                    );

                    return kAbandonParsing;
                }

				node = cursor.CurrentNode();
				if (node.GetType() != kIdentifierName)
				{
					parsedDoc.errors.emplace_back(
						"Expected second named child of metatable arguments node "
						"to be an identifier", GetNodeSourceLocation(node));

					return kAbandonParsing;
				}

                parsedDoc.metaTableName = node.GetText();

                return kContinueParsing;
            }

		} while (cursor.GoToNextSibling());

        cursor.GoToParent();

    } while (cursor.GoToNextSibling());

	parsedDoc.errors.emplace_back(
        "Failed to parse metatable function call and arguments");

    return kAbandonParsing;
}

bool ParseSignaturesTable(TreeCursor& cursor, ParsedLuaSourceDocument& parsedDoc)
{
	if (!cursor.GoToFirstChild())
	{
		parsedDoc.errors.emplace_back("Failed to go to first child of root cursor");
		return kAbandonParsing;
	}

	do // loop through all root children (mostly variable_declaration nodes)
	{ 
		if (cursor.CurrentNode().GetType() != kVariableDeclarationName)
		{
			continue;
		}

		if (!cursor.GoToFirstNamedChild())
		{
			continue;
		}

		// Find 'assignment_statement' child of this variable_declaration
		bool foundAssignment = false;
		do
		{
			if (cursor.CurrentNode().GetType() == kAssignmentStatementName)
			{
				foundAssignment = true;
				break;
			}
		} while (cursor.GoToNextNamedSibling());

		if (!foundAssignment)
		{
			cursor.GoToParent(); // back to variable_declaration
			continue;
		}

		// Now cursor is on assignment_statement. Check if its variable_list contains our metatable name.
		if (!cursor.GoToFirstNamedChild())
		{
			cursor.GoToParent(); // back to variable_declaration
			continue;
		}

		// Search for variable_list
		bool foundVariableList = false;
		do
		{
			if (cursor.CurrentNode().GetType() == kVariableListName)
			{
				foundVariableList = true;
				break;
			}
		} while (cursor.GoToNextNamedSibling());

		if (!foundVariableList)
		{
			cursor.GoToParent(2); // back to variable_declaration
			continue;
		}

		// Now cursor is on variable_list. Search for identifier matching metaTableName.
		if (!cursor.GoToFirstNamedChild())
		{
			cursor.GoToParent(2); // back to variable_declaration
			continue;
		}

		bool foundMetaTableIdentifier = false;
		do
		{
			if (!parsedDoc.metaTableName.empty() && 
				NodeMatchesMetaTableName(cursor.CurrentNode(), parsedDoc.metaTableName))
			{
				foundMetaTableIdentifier = true;
				break;
			}
		} while (cursor.GoToNextNamedSibling());

		if (!foundMetaTableIdentifier)
		{
			cursor.GoToParent(2); // variable_list -> assignment_statement
			cursor.GoToParent();  // assignment_statement -> variable_declaration
			continue;
		}

		// Found the metatable identifier. Now go back to assignment_statement and find expression_list.
		cursor.GoToParent(2); // identifier -> variable_list -> assignment_statement

		if (!cursor.GoToFirstNamedChild())
		{
			cursor.GoToParent(); // back to variable_declaration
			continue;
		}

		// Find expression_list sibling in assignment_statement
		bool foundExpressionList = false;
		do
		{
			if (cursor.CurrentNode().GetType() == kExpressionListName)
			{
				foundExpressionList = true;
				break;
			}
		} while (cursor.GoToNextNamedSibling());

		if (!foundExpressionList)
		{
			cursor.GoToParent(); // back to variable_declaration
			continue;
		}

		// Cursor is now on expression_list. Search for table_constructor.
		if (!cursor.GoToFirstNamedChild())
		{
			cursor.GoToParent(2); // back to variable_declaration
			continue;
		}

		bool foundTableConstructor = false;
		do
		{
			if (cursor.CurrentNode().GetType() == kTableConstructorName)
			{
				foundTableConstructor = true;
				break;
			}
		} while (cursor.GoToNextNamedSibling());

		if (!foundTableConstructor)
		{
			cursor.GoToParent(2); // back to variable_declaration
			continue;
		}

		// Cursor is on table_constructor. Search for 'field' children.
		if (!cursor.GoToFirstNamedChild())
		{
			cursor.GoToParent(3); // back to variable_declaration
			continue;
		}

		bool foundField = false;
		do
		{
			if (cursor.CurrentNode().GetType() == kFieldName)
			{
				// Found a field. Check if it starts with __signatures identifier.
				if (cursor.GoToFirstNamedChild())
				{
					if (IsSignaturesNode(cursor.CurrentNode()))
					{
						// Found __signatures identifier. Now find the sibling table_constructor.
						if (cursor.GoToNextNamedSibling())
						{
							if (cursor.CurrentNode().GetType() == kTableConstructorName)
							{
								// This is the __signatures table! Parse it.
								return ParseSignatureTableFunctionAndArguments(cursor, parsedDoc);
							}
						}
					}
					cursor.GoToParent(); // back to field
				}
			}
		} while (cursor.GoToNextNamedSibling());

		// If we reach here, we didn't find __signatures in this variable_declaration.
		// Go back to variable_declaration and continue with the next one.
		cursor.GoToParent(3); // field/table_constructor -> expression_list -> assignment_statement -> variable_declaration

	} while (cursor.GoToNextSibling());

	parsedDoc.errors.emplace_back("Could not parse signatures table");
	return kContinueParsing;
}

bool ParseTableMemberFunctions(TreeCursor& cursor, ParsedLuaSourceDocument& parsedDoc)
{
    if (!cursor.GoToFirstChild())
    {
        parsedDoc.errors.emplace_back("Failed to go to first child of root cursor");

        return kAbandonParsing;
    }

    do
    {
        if (cursor.CurrentNode().GetType() != kFunctionDeclarationName)
        {
            continue;
		}

        if (!cursor.GoToFirstNamedChild())
        {
            continue;
        }

        SourceLocation functionDeclLocation = GetNodeSourceLocation(cursor.CurrentNode());

        do // find 'dot_index_expression'
        {
            if (cursor.CurrentNode().GetType() != kDotIndexExpressionName)
            {
                continue;
            }

			auto fnNameResult = GetDotIndexExpressionFunctionName(cursor, parsedDoc);
			if (!fnNameResult.Success())
			{
				parsedDoc.errors.emplace_back(
					fnNameResult.GetError().GetMessage(),
					GetNodeSourceLocation(cursor.CurrentNode()));

				continue;
			}

            const auto& fnName = fnNameResult.GetValue();
			if (fnName.empty())
			{
				continue;
			}

			auto fIt = std::ranges::find(parsedDoc.functionNames, fnName);
            if (fIt == parsedDoc.functionNames.end())
            {
                parsedDoc.errors.emplace_back(
                    std::format("Function '{}' not found in function names", fnName),
                    GetNodeSourceLocation(cursor.CurrentNode()));

                continue;
			}

			const auto fnDataIdx = std::distance(parsedDoc.functionNames.begin(), fIt);

			//auto it = parsedDoc.functionDataIndex.find(fnName);
			//if (it == parsedDoc.functionDataIndex.end())
			//{
			//	parsedDoc.errors.emplace_back(
			//		std::format("Function '{}' not found in index", fnName),
			//		GetNodeSourceLocation(cursor.CurrentNode()));

			//	continue;
			//}
			// const auto fnDataIdx = it->second;

			if (fnDataIdx >= parsedDoc.functionParameterNames.size())
			{
				parsedDoc.errors.emplace_back(
					std::format("Invalid function index {} for function '{}'", 
                        fnDataIdx, fnName));

				continue;
			}

            // store the location of this whole function declaration node
            if (fnDataIdx >= parsedDoc.functionBlockInfo.size())
            {
                parsedDoc.errors.emplace_back(
                    std::format("Invalid function index {} for function block info '{}'", 
                        fnDataIdx, fnName));

                continue;
			}

			parsedDoc.functionBlockInfo[fnDataIdx].location = functionDeclLocation;

			if (parsedDoc.functionArgumentTypes.size() != parsedDoc.functionParameterNames.size())
			{
				parsedDoc.errors.emplace_back(
					"Internal error: functionArgumentTypes and "
                    "functionParameterNames size mismatch");

				continue;
			}

			auto& paramNames = parsedDoc.functionParameterNames[fnDataIdx];

			// At this point, cursor is positioned at dot_index_expression (from GetDotIndexExpressionFunctionName).
			// Find the parameters sibling node.
			if (cursor.GoToNextNamedSibling())
			{
				do // loop through siblings looking for 'parameters'
				{
					if (cursor.CurrentNode().GetType() == kParametersName)
					{
						if (cursor.GoToFirstNamedChild())
						{
							do // store parameters
							{
								auto node = cursor.CurrentNode();
								if (node.GetType() == kIdentifierName)
								{
									auto paramText = node.GetText();
									if (!paramText.empty())
									{
										paramNames.emplace_back(paramText);
									}
									else
									{
										parsedDoc.errors.emplace_back(
											"Parameter name is empty",
											GetNodeSourceLocation(node));
									}
								}
							} while (cursor.GoToNextNamedSibling());

							cursor.GoToParent(); // back to parameters node
						}

						auto& argTypes = parsedDoc.functionArgumentTypes[fnDataIdx];

						if (paramNames.size() != argTypes.size())
						{
							parsedDoc.errors.emplace_back(
								std::format("Parameter/argument count mismatch for function '{}' "
								"parameter count: '{}', argument type count '{}'",
								fnName, paramNames.size(), argTypes.size()),
								GetNodeSourceLocation(cursor.CurrentNode()));
						}

						break; // found parameters; don't need to search further siblings
					}
				} while (cursor.GoToNextNamedSibling());

				// After searching for parameters, return cursor to dot_index_expression so  outer loop
				// can continue searching for other dot_index_expression siblings
				cursor.GoToParent(); // from current position (parameters or last searched sibling) back to function_declaration

				if (!cursor.GoToFirstNamedChild())
				{
					continue; // couldn't get back to function_declaration children
				}

				do
				{
					if (cursor.CurrentNode().GetType() == kDotIndexExpressionName)
					{
						break; // positioned at dot_index_expression
					}
				} while (cursor.GoToNextNamedSibling());
			}

        } while (cursor.GoToNextNamedSibling());

        cursor.GoToParent();

	} while (cursor.GoToNextSibling());

    return kContinueParsing;
}

ParsedLuaSourceDocument ParseLuaSourceDocument(TreeCursor cursor)
{
	ParsedLuaSourceDocument parsedDoc{};

	if (!cursor)
	{
		parsedDoc.errors.emplace_back("Failed to parse source string");
		return parsedDoc;
	}

	ParseMetaAndFunctionTableNames(cursor, parsedDoc);

	cursor.Reset();

	ParseSignaturesTable(cursor, parsedDoc);

	cursor.Reset(); 

	ParseTableMemberFunctions(cursor, parsedDoc);

    // sort blocks by location 
    std::ranges::sort(parsedDoc.functionBlockInfo, std::less<>{});

	if (!parsedDoc.functionArgumentTypes.empty())
	{
		if (parsedDoc.functionArgumentTypes.size() != parsedDoc.functionParameterNames.size())
		{
			parsedDoc.errors.emplace_back(
				std::format("Internal error: functionArgumentTypes ({}) and "
                    "functionParameterNames ({}) size mismatch",
					parsedDoc.functionArgumentTypes.size(),
					parsedDoc.functionParameterNames.size()));
		}
		if (parsedDoc.functionDataIndex.size() != parsedDoc.functionArgumentTypes.size())
		{
			parsedDoc.errors.emplace_back(
				std::format("Internal error: functionDataIndex ({}) and "
                    "functionArgumentTypes ({}) size mismatch",
					parsedDoc.functionDataIndex.size(),
					parsedDoc.functionArgumentTypes.size()));
		}
	}

	return parsedDoc;
}

} // unnamed

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

LuaSourceParser::LuaSourceParser() : parser_(CreateUniqueTSParserPtr()), tree_(nullptr)
{
    const auto* language = tree_sitter_lua();

    const bool success = ts_parser_set_language(parser_.get(), language);

    assert(success);
}

void LuaSourceParser::Parse(std::string source)
{
    Reset();

    tree_ = CreateUniqueTSTreePtr(
        parser_.get(),
        nullptr,
        source);

	source_ = std::move(source);
	sourceDocument_ = ParseDocument();
}

ParsedLuaSourceDocument LuaSourceParser::ParseDocument()
{
    if (!tree_ || source_.empty())
    {
        return {};
    }

    return ParseLuaSourceDocument(TreeCursor(ts_tree_root_node(tree_.get()), source_));
}

TreeCursor LuaSourceParser::GetCursorAtByteOffset(size_t byteOffset) const
{
    if (!tree_ || byteOffset < source_.size())
    {
        return {};
    }

    return TreeCursor(ts_node_descendant_for_byte_range(
        GetRootNode(),
        byteOffset,
        byteOffset), source_);
}

void LuaSourceParser::EditSource(std::string&& newSource)
{
    if (!tree_ || source_.empty())
    {
        return;
    }

    auto tsEdit = MakeTSInputEdit(source_, newSource);

    ts_tree_edit(tree_.get(), &tsEdit);

    Reset();

    tree_ = CreateUniqueTSTreePtr(
        parser_.get(),
        tree_.get(),
		newSource);

	source_ = std::move(newSource);
    sourceDocument_ = ParseDocument();
}

bool LuaSourceParser::HasErrors() const
{
    if (!tree_ || source_.empty())
    {
        return false;
    }

    return ts_node_has_error(ts_tree_root_node(tree_.get()));
}

void LuaSourceParser::Reset()
{
    tree_.reset();
    source_ = {};
	sourceDocument_ = {};
}

std::optional<size_t> LuaSourceParser::GetEnclosingFunctionDataIndex(size_t byteOffset) const
{
    auto cursor = GetCursorAtByteOffset(byteOffset);

    while (cursor.GoToParent())
    {
        if (cursor.CurrentNode().GetType() != kFunctionDeclarationName)
        {
            continue;
        }

        if (!cursor.GoToFirstChild())
        {
            continue;
        }

        do 
        {
            if (cursor.CurrentNode().GetType() != kDotIndexExpressionName)
            {
                continue;
            }

            return GetDotIndexExpressionFunctionDataIndex(cursor, sourceDocument_);

        } while (cursor.GoToNextNamedSibling());

        cursor.GoToParent();
    }

    return std::nullopt;
}

TreeNode LuaSourceParser::GetNodeAtByteOffset(size_t byteOffset) const
{
    if (!tree_ || source_.empty())
    {
        return {};
	}

    return TreeNode(ts_node_descendant_for_byte_range(
        GetRootNode(),
        byteOffset,
        byteOffset), source_);
}

TreeNode LuaSourceParser::GetNodeAtPoint(TSPoint point)
{
    return TreeNode(ts_node_descendant_for_point_range(
            GetRootNode(),
            point,
            point), source_);
}

TreeNode LuaSourceParser::GetRootNode() const
{
    return TreeNode(ts_tree_root_node(tree_.get()), source_);
}

void LuaSourceParser::PrintSyntaxTree()
{
    if (!tree_ || source_.empty())
    {
        std::cout << "No syntax tree to print.\n";
        return;
    }

	PrintSyntaxTreeImpl(ts_tree_root_node(tree_.get()), source_, 0);
}

void LuaSourceParser::PrintSyntaxTreeImpl(TSNode node, std::string_view source, int depth)
{
    const char* type = ts_node_type(node);

    const uint32_t start = ts_node_start_byte(node);
    const uint32_t end = ts_node_end_byte(node);

    std::cout << std::string(depth * 2, '-')
        << type
        << " [" << start << ", " << end << "]";

    if (end >= start && end <= source.size())
    {
        std::cout << " `"
            << source.substr(start, end - start)
            << '`';
    }

    if (ts_node_is_error(node))
        std::cout << " [ERROR]";

    if (ts_node_is_missing(node))
        std::cout << " [MISSING]";

    std::cout << '\n';

    const uint32_t count = ts_node_child_count(node);

    for (uint32_t i = 0; i < count; ++i)
    {
        PrintSyntaxTreeImpl(
            ts_node_child(node, i),
            source,
            depth + 1
        );
    }
}

bool ui::TreeCursor::GoToParent(size_t lvls)
{
    for (size_t i = 0; i < lvls; ++i)
    {
        if (!GoToParent())
        {
            return false;
        }
	}

	return true;
}

bool TreeCursor::GoToFirstNamedChild()
{
    if (!GoToFirstChild())
    {
        return false;
    }
    do
    {
        if (CurrentNode().IsNamed())
        {
            return true;
        }
    } while (GoToNextSibling());

    return false;
}

bool TreeCursor::GoToNextNamedSibling()
{
    if (!GoToNextSibling())
    {
        return false;
	}
    do
    {
        if (CurrentNode().IsNamed())
        {
            return true;
        }
    } while (GoToNextSibling());

	return false;
}

void ParsedLuaSourceDocument::Print() const
{
    std::cout << "metatable name: " << metaTableName << '\n'
              << "function table name: " << functionTableName << '\n';

    for (const auto& [fnName, idx] : functionDataIndex)
    {
        std::cout << "function name: " << fnName << '\n';
        if (idx < functionArgumentTypes.size())
        {
            const auto& argTypes = functionArgumentTypes[idx];
            std::cout << "  argument types: ";
            for (const auto& argType : argTypes)
            {
                std::cout << argType << ", ";
            }
            std::cout << '\n';
        }
        if (idx < functionParameterNames.size())
        {
            const auto& paramNames = functionParameterNames[idx];
            std::cout << "  parameter names: ";
            for (const auto& paramName : paramNames)
            {
                std::cout << paramName << ", ";
            }
            std::cout << '\n';
		}
    }

    for (const auto& error : errors)
    {
        std::cout << "error: " << error.message
                  << " at [" << error.location.start
			      << ", " << error.location.end << "]\n";
    }
}

} // ui

#endif