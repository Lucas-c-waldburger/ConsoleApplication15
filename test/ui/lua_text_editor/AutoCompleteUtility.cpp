#include "AutoCompleteUtility.h"

#if IMGUI_ENABLED
#include "TSEditorBridge.h"
#include <ctre/ctre.hpp>
#include "TextDecorationUtility.h"

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

	// pick the innermost block containing the cursor (latest start wins for nested blocks)
	size_t result = std::numeric_limits<size_t>::max();
	size_t bestStart = 0;
	for (const auto& candidate : functionBlocks)
	{
		if (cursorByteOffset >= candidate.location.start &&
			cursorByteOffset <= candidate.location.end &&
			(result == std::numeric_limits<size_t>::max() || candidate.location.start >= bestStart))
		{
			result = candidate.dataIndex;
			bestStart = candidate.location.start;
		}
	}

	return result;
}

bool DoesAutoFillSelectionHaveArguments(std::string_view autoFillStr,
										const ParsedLuaUserTypeInfo& userTypeInfo)
{
	if (autoFillStr.empty())
	{
		return false;
	}
	if (auto dataIdx = userTypeInfo.GetDataIndex(GetRawTypeName(autoFillStr)); 
		dataIdx.IsValid())
	{
		return !userTypeInfo.GetAllUserTypeFieldArgumentTypeIds()[dataIdx].empty();
	}

	return false;
}

} // unnamed

bool AutoCompleteUtility::Draw(ResourceContext& ctx)
{
	if (ctx.textEditor.IsReadOnly() || !ctx.textEditor.IsGhostTextVisible())
	{
		ctx.textEditor.SetAllKeysHandled(true);

		return false;
	}
	if (!autoCompleteFillOptionsPopup_.IsPopupInsideWindowBounds(ctx))
	{
		Reset(ctx.textEditor);

		return false;
	}

	OverrideTextEditorKeyInputs(ctx.textEditor);

	assert(autoCompleteFillOptionsPopup_.selectedIndex < 
		   autoCompleteFillOptionsPopup_.options.size());
	assert((autoCompleteFillOptionsPopup_.popupMenuItemSize.x > 0.0f && 
		    autoCompleteFillOptionsPopup_.popupMenuItemSize.y > 0.0f));

	autoCompleteFillOptionsPopup_.UpdatePopupPosition(ctx);

	ImGui::SetNextWindowSize(autoCompleteFillOptionsPopup_.popupMenuItemSize, ImGuiCond_Appearing);
	ImGui::SetNextWindowPos(autoCompleteFillOptionsPopup_.popupMenuItemPos, ImGuiCond_Always);

	static constexpr ImGuiWindowFlags popupFlags =
		ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoNav |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoMove;

	size_t prevSelectionIndex = autoCompleteFillOptionsPopup_.selectedIndex;
	bool committed = false;

	if (ImGui::BeginPopup("AutoCompletePopup", popupFlags))
	{
		for (size_t i = 0; i < autoCompleteFillOptionsPopup_.options.size(); ++i)
		{
			ImGui::PushID(static_cast<int>(i));

			const bool selected = i == autoCompleteFillOptionsPopup_.selectedIndex;

			if (ImGui::MenuItem(autoCompleteFillOptionsPopup_.options[i].text.c_str(), 
								nullptr, selected))
			{
				autoCompleteFillOptionsPopup_.selectedIndex = i;
				committed = true;

				ImGui::CloseCurrentPopup();
			}

			ImGui::PopID();
		}

		ImGui::EndPopup();
	}

	if (committed || ImGui::IsKeyPressed(ImGuiKey_Tab))
	{
		assert(autoCompleteFillOptionsPopup_.selectedIndex <
			   autoCompleteFillOptionsPopup_.options.size());

		if (committed)
		{
			SetGhostTextFromCurrentSelection(ctx.textEditor);
		}

		using Option = AutoCompleteFillOptionsPopup::Option;

		const auto& option = autoCompleteFillOptionsPopup_.
			options[autoCompleteFillOptionsPopup_.selectedIndex];

		if (option.flags & Option::IsMethod)
		{
			const int cursorBackPlacement = (option.flags & Option::HasArguments) ? 1 : 0;
			ctx.textEditor.CommitGhostText("()", cursorBackPlacement);
		}
		else
		{
			ctx.textEditor.CommitGhostText();
		}

		Reset(ctx.textEditor);

		stateFlags_ |= StateFlags::GhostTextCommitted;

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
		--autoCompleteFillOptionsPopup_;
	}
	else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
	{
		++autoCompleteFillOptionsPopup_;
	}

	const bool newSelectionMadeViaArrowKeys = 
		ctx.textEditor.IsGhostTextVisible() &&
		((stateFlags_ & StateFlags::FillOptionsRegenerated) == 0) &&
		prevSelectionIndex != autoCompleteFillOptionsPopup_.selectedIndex;

	if (newSelectionMadeViaArrowKeys)
	{
		SetGhostTextFromCurrentSelection(ctx.textEditor);
	}

	return false;
}

void AutoCompleteUtility::Cleanup(ResourceContext& ctx)
{
	ctx.textEditor.SetAllKeysHandled(true);

	if (stateFlags_ & StateFlags::GhostTextCommitted)
	{
		ctx.textEditor.SetIsTextChanged(true);
	}

	stateFlags_ = 0;
}

const ObjectFieldAutoCompleteData& 
AutoCompleteUtility::ReparseAutoCompleteData(std::string_view newSource, const TextEditor& textEditor)
{
	lastAutoCompleteData_ = {};

	const auto cursorByteOffset = TextPositionToByteOffset(newSource,
														   textEditor.GetCursorPosition(),
														   textEditor.GetTabSize());
	if (cursorByteOffset <= newSource.size())
	{
		lastAutoCompleteData_ = ObjectFieldAutoCompleteData::Parse(newSource, cursorByteOffset);
	}

	return lastAutoCompleteData_;
}

void AutoCompleteUtility::UpdateForNewText(ResourceContext& ctx)
{
	Reparse(ctx);

	if (lastAutoCompleteData_.IsValid() && autoCompleteFillOptionsPopup_.HasSelection())
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
	assert(cursorByteOffset <= ctx.parser.GetSourceString().size());
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
	assert(autoCompleteFillOptionsPopup_.HasSelection());
	assert(lastAutoCompleteData_.IsValid());

	const auto& option = 
		autoCompleteFillOptionsPopup_.options[autoCompleteFillOptionsPopup_.selectedIndex];

	const size_t existingLen = std::min(
		option.text.size(),
		lastAutoCompleteData_.expressionEndByte - lastAutoCompleteData_.expressionStartByte);

	auto ghostText = std::string_view{ option.text }.substr(existingLen);

	const auto pos = (textEditor.IsGhostTextVisible())
		? textEditor.GetGhostText().position
		: textEditor.GetCursorPosition();

	textEditor.SetGhostText(pos, ghostText);
}

void AutoCompleteUtility::Reparse(ResourceContext& ctx)
{
	Reset(ctx.textEditor);

	const auto cursorByteOffset = GetCursorByteOffset(ctx);
	if (cursorByteOffset > ctx.parser.GetSourceString().size())
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

	auto fillOptions = GetUpdatedFillOptions(cursorByteOffset, autoCompData, 
											 userTypeDataIdx, ctx);

	stateFlags_ |= StateFlags::FillOptionsRegenerated;

	if (fillOptions.empty())
	{
		return;
	}

	autoCompleteFillOptionsPopup_.AssignOptions(std::move(fillOptions));
	autoCompleteFillOptionsPopup_.SetInitialPopupSize();
	autoCompleteFillOptionsPopup_.SetInitialPopupPosition(autoCompData, ctx);

	lastAutoCompleteData_ = std::move(autoCompData);
}

auto AutoCompleteUtility::GetUpdatedFillOptions(
	size_t cursorByteOffset, 
	const ObjectFieldAutoCompleteData& autoCompleteData,
	ParsedLuaUserTypeInfo::DataIndex userTypeDataIdx,
	const ResourceContext& ctx) -> std::vector<AutoCompleteFillOptionsPopup::Option>
{
	using Option = AutoCompleteFillOptionsPopup::Option;

	assert(userTypeDataIdx);
	assert(userTypeDataIdx < ctx.userTypeInfo.GetAllUserTypeFieldNames().size());

	const auto& userTypeFieldNames = ctx.userTypeInfo.GetAllUserTypeFieldNames()[userTypeDataIdx];
	const auto& userTypeMemberKinds = ctx.userTypeInfo.GetAllUserTypeFieldMemberKinds()[userTypeDataIdx];
	const auto& userTypeArgumentTypeIds = ctx.userTypeInfo.GetAllUserTypeFieldArgumentTypeIds()[userTypeDataIdx];

	assert(userTypeFieldNames.size() == userTypeMemberKinds.size());
	assert(userTypeFieldNames.size() == userTypeArgumentTypeIds.size());

	if (userTypeFieldNames.empty())
	{
		return {};
	}

	const auto fullExpr = autoCompleteData.GetFullExpressionText(ctx.parser.GetSourceString());
	const auto partialExpr = autoCompleteData.GetPartialExpressionText(ctx.parser.GetSourceString(),
																	   cursorByteOffset);
	std::vector<Option> fillOptions;

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

				auto& newOption = fillOptions.emplace_back(fieldName);
				if (IsMemberKindFunction(userTypeMemberKinds[i]))
				{
					newOption.flags |= Option::IsMethod;

					if (!userTypeArgumentTypeIds[i].empty())
					{
						newOption.flags |= Option::HasArguments;
					}
				}
			}
		}
	}

	std::ranges::sort(fillOptions, [](const auto& a, const auto& b) {
		return a.text < b.text;
	});

	return fillOptions;
}

ObjectFieldAutoCompleteData
AutoCompleteUtility::GetAutoCompleteData(size_t cursorByteOffset, const ResourceContext& ctx)
{
	return ObjectFieldAutoCompleteData::Parse(ctx.parser.GetSourceString(), cursorByteOffset);
}

void AutoCompleteUtility::Reset(TextEditor& textEditor)
{
	autoCompleteFillOptionsPopup_.Clear();
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
	return cursorByteOffset <= ctx.parser.GetSourceString().size() &&
		   cursorByteOffset > autoCompleteData.parameterEndByte &&
		   cursorByteOffset <= autoCompleteData.expressionEndByte;
}

void AutoCompleteUtility::AutoCompleteFillOptionsPopup::SetInitialPopupSize()
{
	auto longestOption = std::ranges::max_element(options,
		[](const auto& a, const auto& b) { return a.text.size() < b.text.size(); });

	const auto& style = ImGui::GetStyle();

	popupMenuItemSize = ImVec2(
		ImGui::CalcTextSize(longestOption->text.data(), 
							longestOption->text.data() + longestOption->text.size()).x +
		style.WindowPadding.x * 2.0f, 100.0f);
}

void AutoCompleteUtility::AutoCompleteFillOptionsPopup::SetInitialPopupPosition(
	const ObjectFieldAutoCompleteData& autoCompleteData, const ResourceContext& ctx)
{
	const auto accessorScreenPos = autoCompleteData.GetAccessorScreenPos(ctx.parser.GetSourceString(),
																		 ctx.textEditor);

	popupMenuItemPos = ImVec2(accessorScreenPos.x, accessorScreenPos.y +
							  ImGui::GetTextLineHeightWithSpacing() * 
							  ctx.textEditor.GetLineSpacing());

	lastScrollPos = ctx.textEditor.GetScroll();
}

void AutoCompleteUtility::AutoCompleteFillOptionsPopup::UpdatePopupPosition(
	const ResourceContext& ctx)
{
	const auto currentScroll = ctx.textEditor.GetScroll();

	popupMenuItemPos.x -= (currentScroll.x - lastScrollPos.x);
	popupMenuItemPos.y -= (currentScroll.y - lastScrollPos.y);

	lastScrollPos = currentScroll;
}

bool AutoCompleteUtility::AutoCompleteFillOptionsPopup::IsPopupInsideWindowBounds(
	const ResourceContext& ctx) const
{
	const ImVec2 windowMin = ctx.textEditor.GetWindowMin();
	const ImVec2 windowMax = ctx.textEditor.GetWindowMax();

	const ImVec2 popupMin = popupMenuItemPos;
	const ImVec2 popupMax = ImVec2(
		popupMin.x + popupMenuItemSize.x,
		popupMin.y + popupMenuItemSize.y
	);

	return popupMin.x >= windowMin.x &&
		   popupMin.y >= windowMin.y &&
		   popupMax.x <= windowMax.x &&
		   popupMax.y <= windowMax.y;
}

} // ui

#endif