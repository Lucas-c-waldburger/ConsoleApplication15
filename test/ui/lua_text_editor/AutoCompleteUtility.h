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

    bool IsValid() const noexcept;

    std::string_view GetParameterName(std::string_view source) const;

    std::string_view GetPartialExpressionText(std::string_view source,
                                                        size_t cursorBytePos) const;

    std::string_view GetFullExpressionText(std::string_view source) const;

    bool MemberKindMatchesAccessor(LuaUsertypeMemberKind kind) const noexcept;

    bool CursorInExpressionRange(size_t cursorBytePos) const noexcept;

	ImVec2 GetAccessorScreenPos(std::string_view source, const TextEditor& textEditor) const;

	static ObjectFieldAutoCompleteData Parse(std::string_view source, size_t cursorBytePos);
};

ObjectFieldAutoCompleteData ParseAutoCompleteData(std::string_view source, size_t cursorBytePos);

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
	//void Cleanup(ResourceContext& ctx);

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

	static std::vector<std::string>
    GetUpdatedStringFillOptions(const ObjectFieldAutoCompleteData& autoCompleteData,
                                ParsedLuaUserTypeInfo::DataIndex userTypeDataIdx,
                                const ResourceContext& ctx);

    static GhostTextRange DrawGhostText(const std::string& autoFillStr, ResourceContext& ctx, 
                                        const ObjectFieldAutoCompleteData& autoCompleteData);

	void CommitText(ResourceContext& ctx, const std::string& autoFillStr);

    static ImVec2 GetPopupContextItemSize(const std::vector<std::string>& stringFillOptions);
    static ImVec2 GetPopupContextItemPos(ImVec2 accessorScreenPos, float lineSpacing);

    size_t HandleArrowKeyPress() const;

    std::vector<std::string> stringFillOptionsPopup_;
	ObjectFieldAutoCompleteData autoCompleteData;
    size_t currentSelectionIndex = 0;
    ImVec2 popupContextItemSize_ = {};
    ImVec2 popupContextItemPos_ = {};
    GhostTextRange ghostTextRange_ = {};
};

} // ui

#endif