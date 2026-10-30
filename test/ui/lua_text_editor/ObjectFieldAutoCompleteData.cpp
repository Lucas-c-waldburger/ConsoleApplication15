#include "ObjectFieldAutoCompleteData.h"

#if IMGUI_ENABLED
#include "TSEditorBridge.h"

namespace ui {

namespace {

bool IsAlphaNumOrUnderscore(char c)
{
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
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
    if (expressionStartByte >= cursorBytePos || cursorBytePos > source.size())
    {
        return {};
    }

    return source.substr(expressionStartByte, cursorBytePos - expressionStartByte);
}

std::string_view ObjectFieldAutoCompleteData::GetFullExpressionText(std::string_view source) const
{
    if (expressionStartByte >= expressionEndByte || expressionEndByte > source.size())
    {
        return {};
    }

    return source.substr(expressionStartByte, expressionEndByte - expressionStartByte);
}

bool ObjectFieldAutoCompleteData::MemberKindMatchesAccessor(LuaUsertypeMemberKind kind) const noexcept
{
    return (accessor == '.' && IsMemberKindField(kind)) ||
           (accessor == ':' && IsMemberKindFunction(kind));
}

bool ObjectFieldAutoCompleteData::CursorInExpressionRange(size_t cursorBytePos) const noexcept
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

} // ui

#endif