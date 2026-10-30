#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include <vector>
#include <string_view>
#include <imgui.h>
#include "LuaSourceParserCommon.h"
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

} // ui

#endif