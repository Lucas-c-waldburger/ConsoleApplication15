#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include <vector>
#include <string_view>
#include <imgui.h>
#include "../../scripting/ParsedLuaUserTypeInfo.h"
#include "../../deps/ImGuiColorTextEdit/TextEditor.h"
#include "ObjectFieldAutoCompleteData.h"

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
    enum StateFlags : uint8_t
    {
        FillOptionsRegenerated = 1 << 0,
		GhostTextCommitted = 1 << 1
	};

    struct AutoCompleteFillOptionsPopup
    {
        struct Option
        {
            enum : uint8_t
            {
                IsMethod = 1 << 0,
                HasArguments = 1 << 1
            };

            std::string text;
            uint8_t flags = 0;
        };

        std::vector<Option> options;
        size_t selectedIndex = 0;
        ImVec2 popupMenuItemSize = {};
        ImVec2 popupMenuItemPos = {};
		ImVec2 lastScrollPos = {};

		bool HasSelection() const { return !options.empty() && selectedIndex < options.size(); }
		void operator++() { selectedIndex = (selectedIndex + 1) % options.size(); }
		void operator--() { selectedIndex = (selectedIndex + options.size() - 1) % options.size(); }

		void Clear() 
        { 
            options.clear(); 
            selectedIndex = 0; 
            popupMenuItemSize = {};
            popupMenuItemPos = {};
			lastScrollPos = {};
        }
        void AssignOptions(std::vector<Option>&& newOptions)
        {
            options = std::move(newOptions);
            selectedIndex = (options.empty()) ? 0 : std::min(selectedIndex, options.size() - 1);
        }

        void SetInitialPopupSize();
        void SetInitialPopupPosition(const ObjectFieldAutoCompleteData& autoCompleteData, 
                                     const ResourceContext& ctx);
		void UpdatePopupPosition(const ResourceContext& ctx);

        bool IsPopupInsideWindowBounds(const ResourceContext& ctx) const;
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

    static std::vector<AutoCompleteFillOptionsPopup::Option>
    GetUpdatedFillOptions(size_t cursorByteOffset, 
                          const ObjectFieldAutoCompleteData& autoCompleteData,
                          ParsedLuaUserTypeInfo::DataIndex userTypeDataIdx,
                          const ResourceContext& ctx);

    static void OverrideTextEditorKeyInputs(TextEditor& textEditor);

    void SetGhostTextFromCurrentSelection(TextEditor& textEditor);

    AutoCompleteFillOptionsPopup autoCompleteFillOptionsPopup_;
    ObjectFieldAutoCompleteData lastAutoCompleteData_;
	uint8_t stateFlags_ = 0;
};



} // ui

#endif