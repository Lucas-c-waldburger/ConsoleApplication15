#include "TextDecorationUtility.h"

#if IMGUI_ENABLED
#include "TSEditorBridge.h"

namespace ui {

namespace {

constexpr ImColor kErrorUnderlineColor = ImColor(255, 0, 0, 225);

} // unnamed

void TextDecorationUtility::UpdateForNewText(const LuaSourceParser& parser, TextEditor& textEditor)
{
	Reset(textEditor);

	DrawErrors(parser, textEditor);
}

void TextDecorationUtility::Reset(TextEditor& textEditor)
{
	textEditor.ClearTextDecorations();
	textEditor.ClearTextTooltips();
}

void TextDecorationUtility::DrawErrors(const LuaSourceParser& parser, TextEditor& textEditor)
{
	const auto& sourceDoc = parser.GetSourceDocument();

	for (const auto& err : sourceDoc.errors)
	{
		const auto errStart = ByteOffsetToCoordinates(parser.GetSourceString(),
							  err.location.start, textEditor.GetTabSize());
		const auto errEnd = ByteOffsetToCoordinates(parser.GetSourceString(),
							err.location.end, textEditor.GetTabSize());

		textEditor.AddTextDecoration(errStart, errEnd,
									 TextEditor::TextDecoration::Type::WavyUnderline,
									 kErrorUnderlineColor);

		if (!err.message.empty())
		{
			textEditor.AddTextTooltip(errStart, err.message);
		}
	}
}

} // ui

#endif