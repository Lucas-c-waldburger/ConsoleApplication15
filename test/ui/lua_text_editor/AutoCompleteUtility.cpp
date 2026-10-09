#include "AutoCompleteUtility.h"

#if IMGUI_ENABLED
#include <ctre/ctre.hpp>
#include <ranges>
#include "LuaSourceParser.h"
#include "TSEditorBridge.h"
#include "../../deps/ImGuiColorTextEdit/TextEditor.h"

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

bool IsAlphaNumOrUnderscore(char c)
{
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

} // unnamed

constexpr bool ObjectFieldAutoCompleteData::IsValid() const noexcept
{
    return (parameterEndByte > parameterStartByte) &&
           (expressionEndByte >= expressionStartByte) &&
           (accessor == '.' || accessor == ':');
}

constexpr std::string_view 
ObjectFieldAutoCompleteData::GetParameterName(std::string_view source) const
{
    if (parameterStartByte >= parameterEndByte || parameterEndByte >= source.size())
    {
        return {};
    }

    return source.substr(parameterStartByte, parameterEndByte - parameterStartByte);
}

constexpr std::string_view 
ObjectFieldAutoCompleteData::GetExpressionText(std::string_view source) const
{
    if (expressionStartByte >= expressionEndByte || expressionEndByte >= source.size())
    {
        return {};
    }

    return source.substr(expressionStartByte, expressionEndByte - expressionStartByte);
}

constexpr bool 
ObjectFieldAutoCompleteData::MemberKindMatchesAccessor(LuaUsertypeMemberKind kind) const noexcept
{
    return (accessor == '.' && IsMemberKindField(kind)) ||
           (accessor == ':' && IsMemberKindFunction(kind));
}

constexpr bool 
ObjectFieldAutoCompleteData::CursorInExpressionRange(size_t cursorBytePos) const noexcept
{
    return IsValid() && cursorBytePos >= expressionStartByte && cursorBytePos <= expressionEndByte + 1;
}

ImVec2 ObjectFieldAutoCompleteData::GetAccessorScreenPos(std::string_view source, 
                                                         const TextEditor& textEditor) const
{
    if (!IsValid())
    {
        return ImVec2(0, 0);
	}

	const auto accessorCoords = ByteOffsetToCoordinates(source, parameterEndByte,
                                                        textEditor.GetTabSize());
    
	return textEditor.CoordinatesToScreenPos(accessorCoords);
}

ObjectFieldAutoCompleteData ParseAutoCompleteData(std::string_view source, size_t cursorBytePos)
{
    ObjectFieldAutoCompleteData result{};

    cursorBytePos = std::min(cursorBytePos, source.size());
    if (cursorBytePos < 1 || std::isblank(static_cast<unsigned char>(source[cursorBytePos - 1])))
    {
        return result;
    }

    result.expressionEndByte = cursorBytePos;
    size_t head = cursorBytePos - 1;

    // Find start of current expression (identifier after accessor)
    while (head > 0 && IsAlphaNumOrUnderscore(source[head]))
    {
        --head;
    }

    // If head is still pointing to an alphanumeric, we've gone one past the start
    if (head > 0 || (head == 0 && IsAlphaNumOrUnderscore(source[0])))
    {
        result.expressionStartByte = (head == 0 && IsAlphaNumOrUnderscore(source[0]))
            ? 0
            : head + 1;
    }
    else
    {
        return result;
    }

    // Check for accessor before the expression
    if (result.expressionStartByte == 0)
    {
        return result;
    }

    head = result.expressionStartByte - 1;

    if (source[head] != '.' && source[head] != ':')
    {
        return result;
    }

    result.accessor = source[head];
    result.parameterEndByte = head;

    if (head == 0)
    {
        return result;
    }

    --head;

    // Find start of parameter name
    while (head > 0 && IsAlphaNumOrUnderscore(source[head]))
    {
        --head;
    }

    result.parameterStartByte = (head == 0 && IsAlphaNumOrUnderscore(source[0]))
        ? 0
        : head + 1;

    return result;
}

void AutoCompleteDisplay::Reset(TextEditor& textEditor)
{
    stringFillOptions_.clear();
    autoCompleteData = {};
    currentSelectionIndex = 0;
    popupContextItemSize_ = {};
    popupContextItemPos_ = {};
	ghostTextRange_ = {};

    textEditor.SetAllKeysHandled(true);
}

void AutoCompleteDisplay::Update(ResourceContext& ctx)
{
    if (ctx.textEditor.IsTextChanged())
    {
        UpdateForNewText(ctx);
    }
    else if (ctx.textEditor.IsCursorPositionChanged())
    {
        UpdateForNewCursorPos(ctx);
    }
}

void AutoCompleteDisplay::UpdateForNewText(ResourceContext& ctx)
{
    Reset(ctx.textEditor);

	const auto cursorByteOffset = GetCursorByteOffset(ctx);
    if (cursorByteOffset >= ctx.parser.GetSourceString().size())
    {
        return;
    }

    const auto autoCompData = GetAutoCompleteData(cursorByteOffset, ctx);
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

    stringFillOptions_ = GetUpdatedStringFillOptions(autoCompData, userTypeDataIdx, ctx);
    if (stringFillOptions_.empty())
    {
        Reset(ctx.textEditor);
        return;
    }
    else
    {
        LOG_INFO("AutoCompleteDisplay::UpdateForNewText() - stringFillOptions_ size: {}", stringFillOptions_.size());
        for (const auto str : stringFillOptions_)
        {
            LOG_INFO(str);
        }
        LOG_INFO("");
    }

    ghostTextRange_.min = ctx.textEditor.GetCursorPosition();

    autoCompleteData = autoCompData;
    const auto accessorScreenPos = autoCompleteData.GetAccessorScreenPos(ctx.parser.GetSourceString(),
                                                                        ctx.textEditor);

    popupContextItemSize_ = GetPopupContextItemSize(stringFillOptions_);
    popupContextItemPos_ = GetPopupContextItemPos(accessorScreenPos);     

    ctx.textEditor.SetKeyHandled(TextEditor::HandledKeys::UpArrow, false);
    ctx.textEditor.SetKeyHandled(TextEditor::HandledKeys::DownArrow, false);
    ctx.textEditor.SetKeyHandled(TextEditor::HandledKeys::RightArrow, false);
    ctx.textEditor.SetKeyHandled(TextEditor::HandledKeys::Tab, false);

    ImGui::OpenPopup("AutoCompletePopup");
}

void AutoCompleteDisplay::UpdateForNewText(ResourceContext&& ctx)
{
	UpdateForNewText(ctx);
}

void AutoCompleteDisplay::UpdateForNewCursorPos(ResourceContext& ctx)
{
    const auto cursorByteOffset = GetCursorByteOffset(ctx);

    if (!autoCompleteData.CursorInExpressionRange(cursorByteOffset))
    {
        UpdateForNewText(ctx);
    }
    // else we dont need to do anything since we're relying on initial min ghost text pos to insert text at
}

//// BEFORE TextEditor.Render()
void AutoCompleteDisplay::Render(ResourceContext& ctx)
{
    if (!IsVisible())
    {
        return;
    }

	const size_t selIdx = std::min(currentSelectionIndex, stringFillOptions_.size() - 1);
	assert(selIdx < stringFillOptions_.size());

	ctx.textEditor.InsertText(stringFillOptions_[selIdx], TextEditor::PaletteIndex::Comment);

    assert((popupContextItemSize_.x > 0.0f && popupContextItemSize_.y > 0.0f));
    ImGui::SetNextWindowSize(popupContextItemSize_, ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(popupContextItemPos_, ImGuiCond_Appearing);

	bool committed = false;

    if (ImGui::BeginPopupContextItem("AutoCompletePopup"))
    {
        for (size_t i = 0; i < stringFillOptions_.size(); ++i)
        {
            const auto strFillOp = stringFillOptions_[i];

            const bool selected = i == currentSelectionIndex;

            if (ImGui::MenuItem(strFillOp.data(), nullptr, selected))
            {
                currentSelectionIndex = i;
                committed = true;

                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::EndPopup();
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Tab))
    {
        committed = true;
    }
    else
    {
        currentSelectionIndex = HandleArrowKeyPress();
    }

    assert(currentSelectionIndex < stringFillOptions_.size());

    if (committed)
    {
        assert(ghostTextRange_.min.has_value());

        // set cursor to original insert pos
		ctx.textEditor.SetCursorPosition(*ghostTextRange_.min);
		ctx.textEditor.InsertText(stringFillOptions_[currentSelectionIndex].data());

        Reset(ctx.textEditor);
    }
    else
    {
        const auto currentCursorPos = ctx.textEditor.GetCursorPosition();

        // set cursor position to where ghost text starts
		ctx.textEditor.SetCursorPosition(*ghostTextRange_.min);
        ctx.textEditor.InsertText(stringFillOptions_[currentSelectionIndex].data(),
                                  TextEditor::PaletteIndex::Comment);

        // log end of ghost text range for deletion during cleanup
        ghostTextRange_.max = ctx.textEditor.GetCursorPosition();

        // put cursor back to original position
		ctx.textEditor.SetCursorPosition(currentCursorPos);

        // reset changed flags since displaying ghost text is an 'invisible' operation
        ctx.textEditor.SetIsTextChanged(false);
        ctx.textEditor.SetIsCursorPositionChanged(false);
    }
}

//// AFTER TextEditor.Render()
void AutoCompleteDisplay::Cleanup(ResourceContext& ctx)
{
    if (!IsVisible())
    {
        return;
	}

	assert(ghostTextRange_.min.has_value() && ghostTextRange_.max.has_value());
	assert(ghostTextRange_.min < ghostTextRange_.max);

    ctx.textEditor.DeleteRange(*ghostTextRange_.min, *ghostTextRange_.max);
}

bool AutoCompleteDisplay::IsVisible() const
{
    return !stringFillOptions_.empty() && autoCompleteData.IsValid() &&
            currentSelectionIndex < stringFillOptions_.size() &&
            ghostTextRange_.min.has_value();
}

size_t AutoCompleteDisplay::GetCursorByteOffset(const ResourceContext& ctx)
{
    return TextPositionToByteOffset(ctx.parser.GetSourceString(),
                                    ctx.textEditor.GetCursorPosition(),
                                    ctx.textEditor.GetTabSize());
}

ObjectFieldAutoCompleteData 
AutoCompleteDisplay::GetAutoCompleteData(size_t cursorByteOffset, const ResourceContext& ctx)
{
    const auto& sourceStr = ctx.parser.GetSourceString();

    return ParseAutoCompleteData(sourceStr, cursorByteOffset);
}

ParsedLuaUserTypeInfo::DataIndex
AutoCompleteDisplay::GetUserTypeDataIndexForCurrentParameter(size_t cursorByteOffset, 
    const ObjectFieldAutoCompleteData& autoCompleteData, const ResourceContext& ctx)
{
    assert(cursorByteOffset < ctx.parser.GetSourceString().size());
    assert(autoCompleteData.IsValid());

    const auto& parsedDoc = ctx.parser.GetSourceDocument();

    const SourceLocationDataIndexPair searchElem{ .location = { cursorByteOffset, cursorByteOffset } };

    auto it = std::lower_bound(parsedDoc.functionBlockInfo.begin(),
                               parsedDoc.functionBlockInfo.end(),
                               searchElem);

    if (it == parsedDoc.functionBlockInfo.end() || it->location.start > cursorByteOffset)
    {
        // not in a function block
        return {};
    }

    const auto fnDataIdx = it->dataIndex;
    assert(fnDataIdx < parsedDoc.functionNames.size());
    
    return { fnDataIdx };
}

std::vector<std::string_view>
AutoCompleteDisplay::GetUpdatedStringFillOptions(const ObjectFieldAutoCompleteData& autoCompleteData,
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

    const auto partialExpr = autoCompleteData.GetExpressionText(ctx.parser.GetSourceString());

    std::vector<std::string_view> stringFillOptions;

    for (size_t i = 0; i < userTypeFieldNames.size(); ++i)
    {
        if (autoCompleteData.MemberKindMatchesAccessor(userTypeMemberKinds[i]))
        {
            const auto& fieldName = userTypeFieldNames[i];

            if (fieldName.starts_with(partialExpr))
            {
                stringFillOptions.emplace_back(fieldName.substr(partialExpr.size()));
            }
        }
    }

    std::ranges::sort(stringFillOptions, [](const auto& a, const auto& b) {
        return a.size() < b.size();
    });

    return stringFillOptions;
}

size_t AutoCompleteDisplay::HandleArrowKeyPress() const
{
    if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
    {
        return (currentSelectionIndex == 0)
            ? stringFillOptions_.size() - 1
            : currentSelectionIndex - 1;
    }
    else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
    {
        return (currentSelectionIndex + 1) % stringFillOptions_.size();
    }

    return currentSelectionIndex;
}

ImVec2 AutoCompleteDisplay::GetPopupContextItemSize(const std::vector<std::string_view>& stringFillOptions)
{
    auto longestWord = std::ranges::max_element(stringFillOptions,
        [](std::string_view a, std::string_view b) { return a.size() < b.size(); });

    return ImVec2(ImGui::CalcTextSize(longestWord->data()).x + 5.0f, 100.0f);
}

ImVec2 AutoCompleteDisplay::GetPopupContextItemPos(ImVec2 accessorScreenPos)
{
    return ImVec2(accessorScreenPos.x, accessorScreenPos.y + 4.0f);
}

} // ui

#endif