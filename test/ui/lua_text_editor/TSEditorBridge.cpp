#include "TSEditorBridge.h"

#if IMGUI_ENABLED

namespace ui {

constexpr TextEdit TextEdit::Calculate(std::string_view oldSource, std::string_view newSource)
{
    if (oldSource == newSource)
    {
        const size_t sz = oldSource.size();

        return { sz, sz, sz };
    }

    const size_t minSize = std::min(oldSource.size(), newSource.size());

    size_t start = 0;
    while (start < minSize && oldSource[start] == newSource[start])
    {
        ++start;
    }

    size_t oldEnd = oldSource.size();
    size_t newEnd = newSource.size();

    while (oldEnd > start && newEnd > start && oldSource[oldEnd - 1] == newSource[newEnd - 1])
    {
        --oldEnd;
        --newEnd;
    }

    return { start, oldEnd, newEnd };
}

size_t TextPositionToByteOffset(std::string_view source,
                                          TextEditor::Coordinates pos, int tabSize)
{
    if (source.empty())
    {
        return 0;
    }

    if (pos.mLine < 0 || pos.mColumn < 0)
    {
        return 0;
    }

    tabSize = std::max(tabSize, 1);

    size_t byteOffset = 0;
    int currentLine = 0;

    // Find the beginning of the requested line.
    while (byteOffset < source.size() && currentLine < pos.mLine)
    {
        if (source[byteOffset] == '\n')
        {
            ++currentLine;
        }

        ++byteOffset;
    }

    // Requested line is past the end of the document.
    if (currentLine != pos.mLine)
    {
        return source.size();
    }

    int visualColumn = 0;

    while (byteOffset < source.size())
    {
        const unsigned char c = static_cast<unsigned char>(source[byteOffset]);

        // End of the requested line.
        if (c == '\n')
        {
            return byteOffset;
        }

        // ColorTextEdit treats '\r' as part of a Windows CRLF line ending.
        // It therefore shouldn't contribute a visible column.
        if (c == '\r' && byteOffset + 1 < source.size() &&
            source[byteOffset + 1] == '\n')
        {
            return byteOffset;
        }

        // Tab advances to the next tab stop.
        if (c == '\t')
        {
            const int tabAdvance = tabSize - (visualColumn % tabSize);

            // If the requested position falls inside the visual
            // space occupied by the tab, the cursor corresponds to
            // the tab's byte position.
            if (pos.mColumn < visualColumn + tabAdvance)
            {
                return byteOffset;
            }

            visualColumn += tabAdvance;
            ++byteOffset;

            continue;
        }

        // If we've reached the requested visual column, this byte
        // is the character at that position.
        if (visualColumn >= pos.mColumn)
        {
            return byteOffset;
        }

        // Determine the UTF-8 sequence length.
        size_t utf8Length = 1;

        if ((c & 0x80) == 0x00)
        {
            // ASCII
            utf8Length = 1;
        }
        else if ((c & 0xE0) == 0xC0)
        {
            utf8Length = 2;
        }
        else if ((c & 0xF0) == 0xE0)
        {
            utf8Length = 3;
        }
        else if ((c & 0xF8) == 0xF0)
        {
            utf8Length = 4;
        }
        else
        {
            // Invalid UTF-8 leading byte. Treat it as one byte,
            // which is also a safe fallback for arbitrary source data.
            utf8Length = 1;
        }

        // Don't walk beyond the source if the UTF-8 sequence is
        // truncated at the end of the string.
        utf8Length = std::min(utf8Length, source.size() - byteOffset);

        byteOffset += utf8Length;
        ++visualColumn;
    }

    // Cursor is at/past the end of the line.
    return byteOffset;
}

int UTF8CharLength(uint8_t c)
{
    if ((c & 0xFE) == 0xFC)
        return 6;
    if ((c & 0xFC) == 0xF8)
        return 5;
    if ((c & 0xF8) == 0xF0)
        return 4;
    else if ((c & 0xF0) == 0xE0)
        return 3;
    else if ((c & 0xE0) == 0xC0)
        return 2;
    return 1;
}

TextEditor::Coordinates ByteOffsetToCoordinates(std::string_view source, size_t byteOffset, int tabSize)
{
    byteOffset = std::min(byteOffset, source.size());

    int line = 0;
    int column = 0;

    for (size_t i = 0; i < byteOffset;)
    {
        const char c = source[i];

        if (c == '\n')
        {
            ++line;
            column = 0;
            ++i;
        }
        else if (c == '\r')
        {
            // Handle CRLF without counting it as a visual column.
            ++i;
        }
        else if (c == '\t')
        {
            column = (column / tabSize + 1) * tabSize;
            ++i;
        }
        else
        {
            const int length = UTF8CharLength(static_cast<uint8_t>(c));
            i += std::min(static_cast<size_t>(length), byteOffset - i);
            ++column;
        }
    }

    return TextEditor::Coordinates(line, column);
}

TSPoint ByteOffsetToTSPoint(std::string_view source, size_t byteOffset)
{
    byteOffset = std::min(byteOffset, source.size());

    TSPoint point{};

    for (size_t i = 0; i < byteOffset; ++i)
    {
        if (source[i] == '\n')
        {
            ++point.row;
            point.column = 0;
        }
        else
        {
            ++point.column;
        }
    }

    return point;
}

TSInputEdit MakeTSInputEdit(std::string_view oldSource, std::string_view newSource)
{
    const TextEdit edit = TextEdit::Calculate(oldSource, newSource);

    return TSInputEdit{
        static_cast<uint32_t>(edit.startByte),
        static_cast<uint32_t>(edit.oldEndByte),
        static_cast<uint32_t>(edit.newEndByte),

        ByteOffsetToTSPoint(oldSource, edit.startByte),
        ByteOffsetToTSPoint(oldSource, edit.oldEndByte),
        ByteOffsetToTSPoint(newSource, edit.newEndByte)
    };
}

} // ui

#endif