#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include <vector>
#include <string_view>
#include <imgui.h>
#include "../../scripting/ParsedLuaUserTypeInfo.h"
#include "../../deps/ImGuiColorTextEdit/TextEditor.h"
#include "AutoCompleteUtility.h"

namespace ui {

class AutoCompleteUtility
{
public:
    struct ResourceContext
    {
        TextEditor& textEditor;
        const LuaSourceParser& parser;
        const ParsedLuaUserTypeInfo& userTypeInfo;
    };

	const ObjectFieldAutoCompleteData& 
    ReparseAutoCompleteData(std::string_view newSource, const TextEditor& textEditor);
	void UpdateForNewText(ResourceContext& ctx);
    void UpdateForNewCursorPos(ResourceContext& ctx);
    bool Draw(ResourceContext& ctx);
    void Cleanup(ResourceContext& ctx);
    void Reset(TextEditor& textEditor);

    const ObjectFieldAutoCompleteData& GetAutoCompleteData() const noexcept;

private:
    struct StringFillOptionsPopup
    {
        std::vector<std::string> options;
        size_t selectedIndex = 0;
        ImVec2 popupMenuItemSize = {};
        ImVec2 popupMenuItemPos = {};

		bool HasSelection() const { return !options.empty() && selectedIndex < options.size(); }
		void operator++() { selectedIndex = (selectedIndex + 1) % options.size(); }
		void operator--() { selectedIndex = (selectedIndex + options.size() - 1) % options.size(); }

		void Clear() { options.clear(); selectedIndex = 0; }
        void AssignStringOptions(std::vector<std::string>&& newStrs)
        {
            options = std::move(newStrs);
            selectedIndex = (options.empty()) ? 0 : std::min(selectedIndex, options.size() - 1);
        }

        void UpdatePopupSize();
        void UpdatePopupPosition(const ObjectFieldAutoCompleteData& autoCompleteData, 
                                 const ResourceContext& ctx);
	};

    void Reparse(ResourceContext& ctx);

    static size_t GetCursorByteOffset(const ResourceContext& ctx);

    static ObjectFieldAutoCompleteData
    GetAutoCompleteData(size_t cursorByteOffset, const ResourceContext& ctx);

	static bool CursorInsideGhostTextRange(size_t cursorByteOffset, const ResourceContext& ctx,
                                           const ObjectFieldAutoCompleteData& autoCompleteData);

    ParsedLuaUserTypeInfo::DataIndex
    GetUserTypeDataIndexForCurrentParameter(size_t cursorByteOffset,
                                            const ObjectFieldAutoCompleteData& autoCompleteData, 
                                            const ResourceContext& ctx);

    static std::vector<std::string>
    GetUpdatedStringFillOptions(size_t cursorByteOffset, 
                                const ObjectFieldAutoCompleteData& autoCompleteData,
                                ParsedLuaUserTypeInfo::DataIndex userTypeDataIdx,
                                const ResourceContext& ctx);

    static void OverrideTextEditorKeyInputs(TextEditor& textEditor);

    void SetGhostTextFromCurrentSelection(TextEditor& textEditor);

    StringFillOptionsPopup stringFillOptionsPopup_;
    ObjectFieldAutoCompleteData lastAutoCompleteData_;
    bool committedGhostText_ = false;
};



} // ui

#endif