#include "AutoCompleteUtility2.h"

#if IMGUI_ENABLED
#include "TSEditorBridge.h"
#include <ctre/ctre.hpp>

namespace ui {

namespace {

constexpr bool IsMemberKindField(LuaUsertypeMemberKind kind) noexcept
{
	using enum LuaUsertypeMemberKind;

	return kind == Field || kind == Property;
}

constexpr bool IsMemberKindFunction(LuaUsertypeMemberKind kind) noexcept
{
	using enum LuaUsertypeMemberKind;

	return kind == Method || kind == Overload;
}

constexpr std::string_view GetRawTypeName(std::string_view typeNameWithCvRefQualifiers)
{
	static constexpr auto regex = ctll::fixed_string{
		R"(^\s*(?:const\s+)?(?<type>[A-Za-z_][A-Za-z0-9_]*)(?:\s+const)?\s*[&*]?\s*$)"
	};

	auto match = ctre::match<regex>(typeNameWithCvRefQualifiers);
	if (!match)
	{
		return {};
	}

	return match.get<"type">().to_view();
}

size_t GetParsedFunctionIndex(size_t cursorByteOffset,
							  const std::vector<SourceLocationDataIndexPair>& functionBlocks)
{
	if (functionBlocks.empty())
	{
		return std::numeric_limits<size_t>::max();
	}

	size_t idx = functionBlocks.size() / 2;
	while (true)
	{
		const auto& candidate = functionBlocks[idx];
		if (cursorByteOffset < candidate.location.start)
		{
			if (idx == 0)
			{
				return std::numeric_limits<size_t>::max();
			}
			--idx;
		}
		else if (cursorByteOffset > candidate.location.end)
		{
			++idx;
			if (idx >= functionBlocks.size())
			{
				return std::numeric_limits<size_t>::max();
			}
		}
		else
		{
			return candidate.dataIndex;
		}
	}
}

} // unnamed

bool AutoCompleteUtility::Draw(ResourceContext& ctx)
{
	if (ctx.textEditor.IsReadOnly() || !ctx.textEditor.IsGhostTextVisible())
	{
		ctx.textEditor.SetAllKeysHandled(true);

		return false;
	}

	OverrideTextEditorKeyInputs(ctx.textEditor);

	assert(stringFillOptionsPopup_.selectedIndex < stringFillOptionsPopup_.options.size());
	assert((stringFillOptionsPopup_.popupMenuItemSize.x > 0.0f && 
		    stringFillOptionsPopup_.popupMenuItemSize.y > 0.0f));

	ImGui::SetNextWindowSize(stringFillOptionsPopup_.popupMenuItemSize, ImGuiCond_Always);
	ImGui::SetNextWindowPos(stringFillOptionsPopup_.popupMenuItemPos, ImGuiCond_Always);

	static constexpr ImGuiWindowFlags popupFlags =
		ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoNav |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoMove;

	size_t prevSelectionIndex = stringFillOptionsPopup_.selectedIndex;
	bool committed = false;

	if (ImGui::BeginPopup("AutoCompletePopup", popupFlags))
	{
		for (size_t i = 0; i < stringFillOptionsPopup_.options.size(); ++i)
		{
			ImGui::PushID(static_cast<int>(i));

			const bool selected = i == stringFillOptionsPopup_.selectedIndex;

			if (ImGui::MenuItem(stringFillOptionsPopup_.options[i].c_str(), nullptr, selected))
			{
				stringFillOptionsPopup_.selectedIndex = i;
				committed = true;

				ImGui::CloseCurrentPopup();
			}

			ImGui::PopID();
		}

		ImGui::EndPopup();
	}

	if (committed || ImGui::IsKeyPressed(ImGuiKey_Tab))
	{
		assert(lastAutoCompleteData_.IsValid());

		if (lastAutoCompleteData_.accessor == ':')
		{
			ctx.textEditor.CommitGhostTextAsMethod();
		}
		else
		{
			ctx.textEditor.CommitGhostText();
		}

		Reset(ctx.textEditor);

		committedGhostText_ = true;

		return true;
	}

	if (ImGui::IsKeyPressed(ImGuiKey_RightArrow) ||
		ImGui::IsKeyPressed(ImGuiKey_Escape))
	{
		ctx.textEditor.ClearGhostText();
		Reset(ctx.textEditor);

		return true;
	}
	else if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
	{
		--stringFillOptionsPopup_;
	}
	else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
	{
		++stringFillOptionsPopup_;
	}

	if (ctx.textEditor.IsGhostTextVisible() && 
		prevSelectionIndex != stringFillOptionsPopup_.selectedIndex)
	{
		assert(stringFillOptionsPopup_.HasSelection());
		assert(lastAutoCompleteData_.IsValid());

		std::string_view autoFillStr =
			stringFillOptionsPopup_.options[stringFillOptionsPopup_.selectedIndex];

		std::string_view ghostText = autoFillStr.substr(
			lastAutoCompleteData_.expressionEndByte - lastAutoCompleteData_.expressionStartByte);

		ctx.textEditor.SetGhostText(ctx.textEditor.GetGhostText().position, ghostText);
	}

	return false;
}

void AutoCompleteUtility::Cleanup(ResourceContext& ctx)
{
	ctx.textEditor.SetAllKeysHandled(true);

	if (committedGhostText_)
	{
		ctx.textEditor.SetIsTextChanged(true);
		committedGhostText_ = false;
	}
}

const ObjectFieldAutoCompleteData& 
AutoCompleteUtility::ReparseAutoCompleteData(std::string_view newSource, const TextEditor& textEditor)
{
	lastAutoCompleteData_ = {};

	const auto cursorByteOffset = TextPositionToByteOffset(newSource,
														   textEditor.GetCursorPosition(),
														   textEditor.GetTabSize());
	if (cursorByteOffset < newSource.size())
	{
		lastAutoCompleteData_ = ObjectFieldAutoCompleteData::Parse(newSource, cursorByteOffset);
	}

	return lastAutoCompleteData_;
}

void AutoCompleteUtility::UpdateForNewText(ResourceContext& ctx)
{
	Reparse(ctx);

	if (lastAutoCompleteData_.IsValid() && stringFillOptionsPopup_.HasSelection())
	{
		SetGhostTextFromCurrentSelection(ctx.textEditor);

		ImGui::OpenPopup("AutoCompletePopup");
	}
}

void AutoCompleteUtility::UpdateForNewCursorPos(ResourceContext& ctx)
{
	const auto cursorByteOffset = GetCursorByteOffset(ctx);

	if (ctx.textEditor.IsGhostTextVisible() &&
		CursorInsideGhostTextRange(cursorByteOffset, ctx, lastAutoCompleteData_))
	{
		return;
	}

	UpdateForNewText(ctx);
}

ParsedLuaUserTypeInfo::DataIndex
AutoCompleteUtility::GetUserTypeDataIndexForCurrentParameter(size_t cursorByteOffset,
	const ObjectFieldAutoCompleteData& autoCompleteData, const ResourceContext& ctx)
{
	assert(cursorByteOffset < ctx.parser.GetSourceString().size());
	assert(autoCompleteData.IsValid());

	const auto& parsedDoc = ctx.parser.GetSourceDocument();

	const size_t parsedFnIdx = GetParsedFunctionIndex(cursorByteOffset, 
													  parsedDoc.functionBlockInfo);
	if (parsedFnIdx >= parsedDoc.functionArgumentTypes.size())
	{
		// not in a function block
		return {};
	}

	const auto& fnArgTypes = parsedDoc.functionArgumentTypes[parsedFnIdx];
	const auto& fnParamNames = parsedDoc.functionParameterNames[parsedFnIdx];
	assert(fnArgTypes.size() == fnParamNames.size());

	const auto paramName = autoCompleteData.GetParameterName(ctx.parser.GetSourceString());

	for (size_t i = 0; i < fnArgTypes.size(); ++i)
	{
		if (fnParamNames[i] != paramName)
		{
			continue;
		}

		const auto rawTypeName = GetRawTypeName(fnArgTypes[i]);
		if (rawTypeName.empty())
		{
			continue;
		}

		const auto userTypeDataIdx = ctx.userTypeInfo.GetDataIndex(rawTypeName);
		if (userTypeDataIdx.IsValid())
		{
			return userTypeDataIdx;
		}
	}

	return {};
}

void AutoCompleteUtility::OverrideTextEditorKeyInputs(TextEditor& textEditor)
{
	textEditor.SetKeyHandled(TextEditor::HandledKeys::Tab, false);
	textEditor.SetKeyHandled(TextEditor::HandledKeys::UpArrow, false);
	textEditor.SetKeyHandled(TextEditor::HandledKeys::DownArrow, false);
}

void AutoCompleteUtility::SetGhostTextFromCurrentSelection(TextEditor& textEditor)
{
	assert(stringFillOptionsPopup_.HasSelection());
	assert(lastAutoCompleteData_.IsValid());

	std::string_view autoFillStr =
		stringFillOptionsPopup_.options[stringFillOptionsPopup_.selectedIndex];

	const std::string_view ghostText = autoFillStr.substr(
		lastAutoCompleteData_.expressionEndByte - 
		lastAutoCompleteData_.expressionStartByte);

	const auto pos = (textEditor.IsGhostTextVisible())
		? textEditor.GetGhostText().position
		: textEditor.GetCursorPosition();

	textEditor.SetGhostText(pos, ghostText);
}

void AutoCompleteUtility::Reparse(ResourceContext& ctx)
{
	Reset(ctx.textEditor);

	const auto cursorByteOffset = GetCursorByteOffset(ctx);
	if (cursorByteOffset >= ctx.parser.GetSourceString().size())
	{
		return;
	}

	auto autoCompData = GetAutoCompleteData(cursorByteOffset, ctx);
	if (!autoCompData.IsValid())
	{
		return;
	}

	const auto userTypeDataIdx = GetUserTypeDataIndexForCurrentParameter(cursorByteOffset,
																		 autoCompData, ctx);
	if (!userTypeDataIdx)
	{
		return;
	}

	auto strFillOptions = GetUpdatedStringFillOptions(cursorByteOffset, autoCompData, 
													  userTypeDataIdx, ctx);
	if (strFillOptions.empty())
	{
		return;
	}

	stringFillOptionsPopup_.AssignStringOptions(std::move(strFillOptions));
	stringFillOptionsPopup_.UpdatePopupSize();
	stringFillOptionsPopup_.UpdatePopupPosition(autoCompData, ctx);

	lastAutoCompleteData_ = std::move(autoCompData);
}

std::vector<std::string>
AutoCompleteUtility::GetUpdatedStringFillOptions(size_t cursorByteOffset, 
												 const ObjectFieldAutoCompleteData& autoCompleteData,
												 ParsedLuaUserTypeInfo::DataIndex userTypeDataIdx,
												 const ResourceContext& ctx)
{
	assert(userTypeDataIdx);
	assert(userTypeDataIdx < ctx.userTypeInfo.GetUserTypeFieldNames().size());

	const auto& userTypeFieldNames = ctx.userTypeInfo.GetUserTypeFieldNames()[userTypeDataIdx];
	const auto& userTypeMemberKinds = ctx.userTypeInfo.GetUserTypeMemberKinds()[userTypeDataIdx];

	assert(userTypeFieldNames.size() == userTypeMemberKinds.size());

	if (userTypeFieldNames.empty())
	{
		return {};
	}

	const auto fullExpr = autoCompleteData.GetFullExpressionText(ctx.parser.GetSourceString());
	const auto partialExpr = autoCompleteData.GetPartialExpressionText(ctx.parser.GetSourceString(),
																	   cursorByteOffset);

	std::vector<std::string> stringFillOptions;

	for (size_t i = 0; i < userTypeFieldNames.size(); ++i)
	{
		if (autoCompleteData.MemberKindMatchesAccessor(userTypeMemberKinds[i]))
		{
			const auto& fieldName = userTypeFieldNames[i];

			if (fieldName.starts_with(partialExpr))
			{
				if (fieldName == fullExpr)
				{
					// already valid expression, turn off autocomplete
					return {};
				}

				stringFillOptions.emplace_back(fieldName);
			}
		}
	}

	std::ranges::sort(stringFillOptions);

	return stringFillOptions;
}

ObjectFieldAutoCompleteData
AutoCompleteUtility::GetAutoCompleteData(size_t cursorByteOffset, const ResourceContext& ctx)
{
	return ObjectFieldAutoCompleteData::Parse(ctx.parser.GetSourceString(), cursorByteOffset);
}

void AutoCompleteUtility::Reset(TextEditor& textEditor)
{
	stringFillOptionsPopup_.Clear();
	lastAutoCompleteData_ = {};
	textEditor.ClearGhostText();
}

const ObjectFieldAutoCompleteData& AutoCompleteUtility::GetAutoCompleteData() const noexcept
{
	return lastAutoCompleteData_;
}

size_t AutoCompleteUtility::GetCursorByteOffset(const ResourceContext& ctx)
{
	return TextPositionToByteOffset(ctx.parser.GetSourceString(),
								    ctx.textEditor.GetCursorPosition(),
								    ctx.textEditor.GetTabSize());
}

bool AutoCompleteUtility::CursorInsideGhostTextRange(size_t cursorByteOffset, const ResourceContext& ctx,
												     const ObjectFieldAutoCompleteData& autoCompleteData)
{
	if (!ctx.textEditor.IsGhostTextVisible())
	{
		return true;
	}

	// check if cursor backed up to before the ':'/'.', or ahead of the non-ghost text expr
	return cursorByteOffset < ctx.parser.GetSourceString().size() &&
		   cursorByteOffset > autoCompleteData.parameterEndByte &&
		   cursorByteOffset < autoCompleteData.expressionEndByte;
}

void AutoCompleteUtility::StringFillOptionsPopup::UpdatePopupSize()
{
	auto longestWord = std::ranges::max_element(options,
		[](std::string_view a, std::string_view b) { return a.size() < b.size(); });

	popupMenuItemSize = ImVec2(ImGui::CalcTextSize(longestWord->data()).x + 
		ImGui::GetStyle().WindowPadding.x + 5.0f, 100.0f);
}

void AutoCompleteUtility::StringFillOptionsPopup::UpdatePopupPosition(const ObjectFieldAutoCompleteData& autoCompleteData, const ResourceContext& ctx)
{
	const auto accessorScreenPos = autoCompleteData.GetAccessorScreenPos(ctx.parser.GetSourceString(),
																		 ctx.textEditor);

	popupMenuItemPos = ImVec2(accessorScreenPos.x, accessorScreenPos.y +
							     ImGui::GetTextLineHeightWithSpacing() * 
								 ctx.textEditor.GetLineSpacing());
}

} // ui

#endif