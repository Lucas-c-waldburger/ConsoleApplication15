#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include <vector>
#include <string_view>
#include <imgui.h>
#include "../../scripting/ParsedLuaUserTypeInfo.h"
#include "../../deps/ImGuiColorTextEdit/TextEditor.h"

namespace ui {

class LuaSourceParser;

struct ObjectFieldAutoCompleteData
{
    size_t parameterStartByte = 0;
    size_t parameterEndByte = 0;
    size_t expressionStartByte = 0;
    size_t expressionEndByte = 0;
    char accessor = '\0';

    constexpr bool IsValid() const noexcept;

    constexpr std::string_view GetParameterName(std::string_view source) const;

    constexpr std::string_view GetExpressionText(std::string_view source) const;

    constexpr bool MemberKindMatchesAccessor(LuaUsertypeMemberKind kind) const noexcept;

    constexpr bool CursorInExpressionRange(size_t cursorBytePos) const noexcept;

	ImVec2 GetAccessorScreenPos(std::string_view source, const TextEditor& textEditor) const;
};

ObjectFieldAutoCompleteData ParseAutoCompleteData(std::string_view source, size_t cursorBytePos);

//std::vector<std::string>
//GetAutoCompleteStringFillOptions(const LuaSourceParser& parser, const ParsedLuaUserTypeInfo& userTypeInfo,
//                                 size_t cursorByteOffset);

class AutoCompleteDisplay
{
public:
    struct ResourceContext
    {
        TextEditor& textEditor;
        const LuaSourceParser& parser;
        const ParsedLuaUserTypeInfo& userTypeInfo;
    };

    void Update(ResourceContext& ctx);
    void UpdateForNewText(ResourceContext& ctx);
    void UpdateForNewText(ResourceContext&& ctx);
    void UpdateForNewCursorPos(ResourceContext& ctx);

    void Render(ResourceContext& ctx);
	void Cleanup(ResourceContext& ctx);

    void Reset(TextEditor& textEditor);

    bool IsVisible() const;

private:
	using GhostTextRange = Range<std::optional<TextEditor::Coordinates>>;

	static size_t GetCursorByteOffset(const ResourceContext& ctx);

    static ObjectFieldAutoCompleteData 
    GetAutoCompleteData(size_t cursorByteOffset, const ResourceContext& ctx);

    static ParsedLuaUserTypeInfo::DataIndex 
    GetUserTypeDataIndexForCurrentParameter(size_t cursorByteOffset, 
        const ObjectFieldAutoCompleteData& autoCompleteData, 
        const ResourceContext& ctx);

	static std::vector<std::string_view>
    GetUpdatedStringFillOptions(const ObjectFieldAutoCompleteData& autoCompleteData,
                                ParsedLuaUserTypeInfo::DataIndex userTypeDataIdx,
                                const ResourceContext& ctx);

    static ImVec2 GetPopupContextItemSize(const std::vector<std::string_view>& stringFillOptions);
    static ImVec2 GetPopupContextItemPos(ImVec2 accessorScreenPos);

    size_t HandleArrowKeyPress() const;

    std::vector<std::string_view> stringFillOptions_;
	ObjectFieldAutoCompleteData autoCompleteData;
    size_t currentSelectionIndex = 0;
    ImVec2 popupContextItemSize_ = {};
    ImVec2 popupContextItemPos_ = {};
    GhostTextRange ghostTextRange_ = {};
};

} // ui

#endif