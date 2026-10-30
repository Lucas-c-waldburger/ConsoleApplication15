#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include <vector>
#include <string_view>
#include <imgui.h>
#include "../../scripting/ParsedLuaUserTypeInfo.h"
#include "../../deps/ImGuiColorTextEdit/TextEditor.h"
#include "ObjectFieldAutoCompleteData.h"
#include "UtilityCommon.h"

namespace ui {

class AutoCompleteUtility
{
public:
	const ObjectFieldAutoCompleteData& 
    ReparseAutoCompleteData(std::string_view newSource, const TextEditor& textEditor);
    void Update(LuaTextEditorResourceContext& ctx);
	void UpdateForNewText(LuaTextEditorResourceContext& ctx);
    void UpdateForNewCursorPos(LuaTextEditorResourceContext& ctx);
    bool Draw(LuaTextEditorResourceContext& ctx);
    void Cleanup(LuaTextEditorResourceContext& ctx);
    void Reset(TextEditor& textEditor);

    const ObjectFieldAutoCompleteData& GetAutoCompleteRegionData() const noexcept;

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
                                     const LuaTextEditorResourceContext& ctx);
		void UpdatePopupPosition(const LuaTextEditorResourceContext& ctx);

        bool IsPopupInsideWindowBounds(const LuaTextEditorResourceContext& ctx) const;
	};

    void Reparse(LuaTextEditorResourceContext& ctx);

    static size_t GetCursorByteOffset(const LuaTextEditorResourceContext& ctx);

    static ObjectFieldAutoCompleteData
    MakeAutoCompleteRegionData(size_t cursorByteOffset, const LuaTextEditorResourceContext& ctx);

	static bool CursorInsideGhostTextRange(size_t cursorByteOffset, const LuaTextEditorResourceContext& ctx,
                                           const ObjectFieldAutoCompleteData& autoCompleteData);

    ParsedLuaUserTypeInfo::DataIndex
    GetUserTypeDataIndexForCurrentParameter(size_t cursorByteOffset,
                                            const ObjectFieldAutoCompleteData& autoCompleteData, 
                                            const LuaTextEditorResourceContext& ctx);

    static std::vector<AutoCompleteFillOptionsPopup::Option>
    GetUpdatedFillOptions(size_t cursorByteOffset, 
                          const ObjectFieldAutoCompleteData& autoCompleteData,
                          ParsedLuaUserTypeInfo::DataIndex userTypeDataIdx,
                          const LuaTextEditorResourceContext& ctx);

    static void OverrideTextEditorKeyInputs(TextEditor& textEditor);

    void SetGhostTextFromCurrentSelection(TextEditor& textEditor);

    AutoCompleteFillOptionsPopup autoCompleteFillOptionsPopup_;
    ObjectFieldAutoCompleteData lastAutoCompleteData_;
	uint8_t stateFlags_ = 0;
};



} // ui

#endif