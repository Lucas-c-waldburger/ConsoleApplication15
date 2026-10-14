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

constexpr char CharAtByteOffset(std::string_view source, size_t byteOffset)
{
    return (byteOffset < source.size()) ? source[byteOffset] : '\0';
}

//size_t GetParameterCallIndex(const ObjectFieldAutoCompleteData& autoCompleteData,
//                             const LuaSourceParser& parser)
//{
//    const auto& sourceStr = parser.GetSourceString();
//
//	const auto paramName = autoCompleteData.GetParameterName(sourceStr);
//
//}

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

std::string_view GetGhostTextDisplaySubstring(const std::string& autoFillStr, 
                                              const ObjectFieldAutoCompleteData& autoCompleteData)
{
	assert(autoCompleteData.expressionStartByte <= autoCompleteData.expressionEndByte);

	return autoFillStr.substr(autoCompleteData.expressionEndByte - 
                              autoCompleteData.expressionStartByte);
}

} // unnamed

bool ObjectFieldAutoCompleteData::IsValid() const noexcept
{
    return (parameterEndByte > parameterStartByte) &&
           (expressionEndByte >= expressionStartByte) &&
           (accessor == '.' || accessor == ':');
}

std::string_view 
ObjectFieldAutoCompleteData::GetParameterName(std::string_view source) const
{
    if (parameterStartByte >= parameterEndByte || parameterEndByte >= source.size())
    {
        return {};
    }

    return source.substr(parameterStartByte, parameterEndByte - parameterStartByte);
}

std::string_view 
ObjectFieldAutoCompleteData::GetPartialExpressionText(std::string_view source, 
                                                      size_t cursorBytePos) const
{
    if (expressionStartByte >= cursorBytePos || cursorBytePos >= source.size())
    {
        return {};
    }

    return source.substr(expressionStartByte, cursorBytePos - expressionStartByte);
}

std::string_view 
ObjectFieldAutoCompleteData::GetFullExpressionText(std::string_view source) const
{
    if (expressionStartByte >= expressionEndByte || expressionEndByte >= source.size())
    {
        return {};
    }

    return source.substr(expressionStartByte, expressionEndByte - expressionStartByte);
}

bool 
ObjectFieldAutoCompleteData::MemberKindMatchesAccessor(LuaUsertypeMemberKind kind) const noexcept
{
    return (accessor == '.' && IsMemberKindField(kind)) ||
           (accessor == ':' && IsMemberKindFunction(kind));
}

bool 
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

ObjectFieldAutoCompleteData 
ObjectFieldAutoCompleteData::Parse(std::string_view source, size_t cursorBytePos)
{
    ObjectFieldAutoCompleteData result{};

    cursorBytePos = std::min(cursorBytePos, source.size());
    if (cursorBytePos < 1 || std::isblank(static_cast<unsigned char>(source[cursorBytePos - 1])))
    {
        return result;
    }
    
    char ch = '\0';

    size_t head = cursorBytePos;
    while (head < source.size() && IsAlphaNumOrUnderscore(source[head]))
    {
        ch = source[head];
        ++head;
    }

    ch = source[head];
    result.expressionEndByte = head;

    head = cursorBytePos - 1;
    ch = source[head];

    // Find start of current expression (identifier after accessor)
    while (head > 0 && IsAlphaNumOrUnderscore(source[head]))
    {
        ch = source[head];
        --head;
    }
    ch = source[head];
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
    ch = source[head];

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
    stringFillOptionsPopup_.clear();
    autoCompleteData = {};
    currentSelectionIndex = 0;
    popupContextItemSize_ = {};
    popupContextItemPos_ = {};
	ghostTextRange_ = {};

    textEditor.SetAllKeysHandled(true);

    LOG_DEBUG("TextEditor Keys ENABLED");
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

    //LOG_WARNING_FMT("AutoCompleteLocation - parameterStartByte: {}, parameterEndByte: {}, expressionStartByte: {}, expressionEndByte: {}, accessor: '{}'",
    //                autoCompData.parameterStartByte, autoCompData.parameterEndByte,
    //                autoCompData.expressionStartByte, autoCompData.expressionEndByte,
		  //          autoCompData.accessor);

    const auto userTypeDataIdx = GetUserTypeDataIndexForCurrentParameter(cursorByteOffset, 
                                                                         autoCompData, ctx);
    if (!userTypeDataIdx)
    {
        return;
	}

    stringFillOptionsPopup_ = GetUpdatedStringFillOptions(autoCompData, userTypeDataIdx, ctx);
    if (stringFillOptionsPopup_.empty())
    {
        Reset(ctx.textEditor);
        return;
    }
    else
    {
        LOG_INFO("AutoCompleteDisplay::UpdateForNewText() - stringFillOptions_ size: {}", stringFillOptionsPopup_.size());
        for (const auto str : stringFillOptionsPopup_)
        {
            LOG_INFO(str);
        }
        LOG_INFO("");
    }

    autoCompleteData = autoCompData;
    const auto accessorScreenPos = autoCompleteData.GetAccessorScreenPos(ctx.parser.GetSourceString(),
                                                                         ctx.textEditor);

	ghostTextRange_ = DrawGhostText(stringFillOptionsPopup_[0], ctx, autoCompleteData);

    popupContextItemSize_ = GetPopupContextItemSize(stringFillOptionsPopup_);
    popupContextItemPos_ = GetPopupContextItemPos(accessorScreenPos, ctx.textEditor.GetLineSpacing());     

    ctx.textEditor.SetIsTextChanged(false);
    ctx.textEditor.SetIsCursorPositionChanged(false);

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

    LOG_ERROR("TextEditor Keys DISABLED");

    ctx.textEditor.SetKeyHandled(TextEditor::HandledKeys::UpArrow, false);
    ctx.textEditor.SetKeyHandled(TextEditor::HandledKeys::DownArrow, false);
    ctx.textEditor.SetKeyHandled(TextEditor::HandledKeys::RightArrow, false);
    ctx.textEditor.SetKeyHandled(TextEditor::HandledKeys::Tab, false);

    assert((popupContextItemSize_.x > 0.0f && popupContextItemSize_.y > 0.0f));
    ImGui::SetNextWindowSize(popupContextItemSize_, ImGuiCond_Always);
    ImGui::SetNextWindowPos(popupContextItemPos_, ImGuiCond_Always);

    static constexpr ImGuiWindowFlags popupFlags =
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoMove;

	size_t prevSelectionIndex = currentSelectionIndex;
	bool committed = false;

    if (ImGui::BeginPopup("AutoCompletePopup", popupFlags))
    {
        for (size_t i = 0; i < stringFillOptionsPopup_.size(); ++i)
        {
			ImGui::PushID(static_cast<int>(i));

            const bool selected = i == currentSelectionIndex;

            if (ImGui::MenuItem(stringFillOptionsPopup_[i].c_str(), nullptr, selected))
            {
                currentSelectionIndex = i;
                committed = true;

                ImGui::CloseCurrentPopup();
            }

            ImGui::PopID();
        }

        ImGui::EndPopup();
    }

   // if (ImGui::BeginPopupContextItem("AutoCompletePopup"))
   // {
   //     for (size_t i = 0; i < stringFillOptionsPopup_.size(); ++i)
   //     {
   //         const auto& strFillOp = stringFillOptionsPopup_[i];

   //         const bool selected = i == currentSelectionIndex;

			////LOG_INFO("LABEL: {}", label);
   //         if (ImGui::MenuItem(strFillOp.c_str(), nullptr, selected))
   //         {
   //             currentSelectionIndex = i;
   //             committed = true;

   //             ImGui::CloseCurrentPopup();
   //         }
   //     }

   //     ImGui::EndPopup();
   // }

    if (ImGui::IsKeyPressed(ImGuiKey_Tab))
    {
        committed = true;
    }
    else
    {
        currentSelectionIndex = HandleArrowKeyPress();
    }

    assert(currentSelectionIndex < stringFillOptionsPopup_.size());

    if (currentSelectionIndex != prevSelectionIndex)
    {
        assert((ghostTextRange_.min.has_value() && ghostTextRange_.max.has_value()));

		ctx.textEditor.DeleteRange(*ghostTextRange_.min, *ghostTextRange_.max);

        ctx.textEditor.InsertText(stringFillOptionsPopup_[currentSelectionIndex], 
                                  TextEditor::PaletteIndex::Comment);

		ghostTextRange_.max = ctx.textEditor.GetCursorPosition();
	}

    if (committed)
    {
        assert((ghostTextRange_.min.has_value() && ghostTextRange_.max.has_value()));

		ctx.textEditor.SetColorRange(*ghostTextRange_.min, *ghostTextRange_.max, 
                                     TextEditor::PaletteIndex::Default);

		ctx.textEditor.SetCursorPosition(*ghostTextRange_.max);

        Reset(ctx.textEditor);
    }
}

bool AutoCompleteDisplay::IsVisible() const
{
    //return !stringFillOptions_.empty() && autoCompleteData.IsValid() &&
    //        currentSelectionIndex < stringFillOptions_.size() &&
    return ghostTextRange_.min.has_value() && ghostTextRange_.max.has_value();
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

    const size_t parsedFnIdx = GetParsedFunctionIndex(cursorByteOffset, parsedDoc.functionBlockInfo);
    if (parsedFnIdx >= parsedDoc.functionArgumentTypes.size())
    {
        // not in a function block
        return {};
    }

	const auto& fnArgTypes = parsedDoc.functionArgumentTypes[parsedFnIdx];
    for (const auto& argType : fnArgTypes)
    {
        const auto rawTypeName = GetRawTypeName(argType);
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

std::vector<std::string>
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

    const auto partialExpr = autoCompleteData.GetFullExpressionText(ctx.parser.GetSourceString());

    std::vector<std::string> stringFillOptions;

    for (size_t i = 0; i < userTypeFieldNames.size(); ++i)
    {
        if (autoCompleteData.MemberKindMatchesAccessor(userTypeMemberKinds[i]))
        {
            const auto& fieldName = userTypeFieldNames[i];

            if (fieldName.starts_with(partialExpr))
            {
                stringFillOptions.emplace_back(fieldName);
            }
        }
    }

    std::ranges::sort(stringFillOptions);

    return stringFillOptions;
}

AutoCompleteDisplay::GhostTextRange 
AutoCompleteDisplay::DrawGhostText(const std::string& autoFillStr,
                                   ResourceContext& ctx, 
                                   const ObjectFieldAutoCompleteData& autoCompleteData)
{
    GhostTextRange ghostTextRange{};

    const auto originalCursorPos = ctx.textEditor.GetCursorPosition();

    ghostTextRange.min = originalCursorPos;

    const auto ghostText = autoFillStr.substr(autoCompleteData.expressionEndByte -
                                              autoCompleteData.expressionStartByte);

    ctx.textEditor.InsertText(ghostText, TextEditor::PaletteIndex::Comment);

    ghostTextRange.max = ctx.textEditor.GetCursorPosition();

    ctx.textEditor.SetCursorPosition(originalCursorPos);

    return ghostTextRange;
}

size_t AutoCompleteDisplay::HandleArrowKeyPress() const
{
    if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
    {
        return (currentSelectionIndex == 0)
            ? stringFillOptionsPopup_.size() - 1
            : currentSelectionIndex - 1;
    }
    else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
    {
        return (currentSelectionIndex + 1) % stringFillOptionsPopup_.size();
    }

    return currentSelectionIndex;
}

ImVec2 AutoCompleteDisplay::GetPopupContextItemSize(const std::vector<std::string>& stringFillOptions)
{
    auto longestWord = std::ranges::max_element(stringFillOptions,
        [](std::string_view a, std::string_view b) { return a.size() < b.size(); });

    return ImVec2(ImGui::CalcTextSize(longestWord->data()).x + 5.0f, 100.0f);
}

ImVec2 AutoCompleteDisplay::GetPopupContextItemPos(ImVec2 accessorScreenPos, float lineSpacing)
{
    return ImVec2(accessorScreenPos.x, accessorScreenPos.y + 
                  ImGui::GetTextLineHeightWithSpacing() * lineSpacing);
}

} // ui

#endif