#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "LuaSourceParser.h"
#include "LuaTextEditor.h"

namespace ui
{

struct TextEdit
{
	size_t startByte = 0;
	size_t oldEndByte = 0;
	size_t newEndByte = 0;

	static constexpr TextEdit Calculate(std::string_view oldSource, std::string_view newSource);
};

size_t TextPositionToByteOffset(std::string_view source,
										  TextEditor::Coordinates pos, int tabSize);

int UTF8CharLength(uint8_t c);

TextEditor::Coordinates ByteOffsetToCoordinates(std::string_view source, size_t byteOffset, int tabSize);

TSPoint ByteOffsetToTSPoint(std::string_view source, size_t byteOffset);

TSInputEdit MakeTSInputEdit(std::string_view oldSource, std::string_view newSource);


} // ui

#endif