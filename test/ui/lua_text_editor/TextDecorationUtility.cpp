#include "TextDecorationUtility.h"

#if IMGUI_ENABLED
#include <ranges>
#include "TSEditorBridge.h"

namespace ui {

namespace {

constexpr ImColor kErrorUnderlineColor = ImColor(255, 0, 0, 225);
constexpr ImColor kWarningUnderlineColor = ImColor(74, 246, 38, 225);
constexpr float kPendingErrorDelay = 0.8f;

struct FunctionBlockTracker
{
	size_t index = 0;

	bool Update(const LuaSourceDocument& doc,
			    const LuaSourceDocument::IndexExpressionInfo& indexExpr)
	{
		if (index >= doc.functionBlocks.size())
		{
			return false;
		}

		if (indexExpr.location.start >= doc.functionBlocks[index].location.end)
		{
			++index;
		}

		return index < doc.functionBlocks.size() &&
			   indexExpr.location.Inside(doc.functionBlocks[index].location);
	}
};

void AddErrorDecoration(const SourceDocumentError& error, const LuaSourceParser& parser, 
						TextEditor& textEditor, ImColor color)
{
	const auto errStart = ByteOffsetToCoordinates(parser.GetSourceString(),
						  error.location.start, textEditor.GetTabSize());
	const auto errEnd = ByteOffsetToCoordinates(parser.GetSourceString(),
						error.location.end, textEditor.GetTabSize());

	textEditor.AddTextDecoration(errStart, errEnd,
								 TextEditor::TextDecoration::Type::WavyUnderline,
								 color);
	if (!error.message.empty())
	{
		textEditor.AddTextTooltip(errStart, error.message);
	}
}

constexpr char IndexExpressionTypeToChar(LuaSourceDocument::IndexExpressionType type)
{
	switch (type)
	{
	case LuaSourceDocument::IndexExpressionType::Dot:
		return '.';
	case LuaSourceDocument::IndexExpressionType::Method:
		return ':';
	default:
		return '\0';
	}
}

constexpr bool IndexExpressionTypeMatchesMemberKind(
	LuaSourceDocument::IndexExpressionType exprType, LuaUsertypeMemberKind memberKind)
{
	using ExprType = LuaSourceDocument::IndexExpressionType;
	using Kind = LuaUsertypeMemberKind;

	return (exprType == ExprType::Dot && 
			(memberKind == Kind::Field || memberKind == Kind::Property)) ||
		   (exprType == ExprType::Method && memberKind == Kind::Method);
}

constexpr std::string_view 
IndexExpressionTypeToString(LuaSourceDocument::IndexExpressionType type)
{
	switch (type)
	{
	case LuaSourceDocument::IndexExpressionType::Dot:
		return "Field";
	case LuaSourceDocument::IndexExpressionType::Method:
		return "Method";
	default:
		return "";
	}
}

std::pair<TextEditor::Coordinates, TextEditor::Coordinates>
GetIndexExpressionRhsCoordinates(const LuaSourceDocument::IndexExpressionInfo& indexExpr,
								const LuaSourceParser& parser, const TextEditor& textEditor)
{
	const char indexingChar = IndexExpressionTypeToChar(indexExpr.type);
	if (indexingChar == '\0')
	{
		return {};
	}

	const std::string_view exprSrc = std::string_view{ parser.GetSourceString() }
		.substr(indexExpr.location.start, indexExpr.location.end - indexExpr.location.start);

	const auto indexingCharPos = exprSrc.find_first_of(indexingChar);
	if (indexingCharPos == std::string_view::npos)
	{
		return {};
	}

	const auto rhsStart = ByteOffsetToCoordinates(parser.GetSourceString(),
						  indexExpr.location.start + indexingCharPos + 1, textEditor.GetTabSize());
	const auto rhsEnd = ByteOffsetToCoordinates(parser.GetSourceString(),
						indexExpr.location.end, textEditor.GetTabSize());

	return { rhsStart, rhsEnd };
}

void DrawIndexExpressionDecorations(const LuaTextEditorResourceContext& ctx)
{
	const auto& doc = ctx.parser.GetSourceDocument();
	if (doc.indexExpressions.empty())
	{
		return;
	}

	FunctionBlockTracker tracker{};

	for (const auto& indexExpr : doc.indexExpressions)
	{
		std::string_view paramType;

		const bool insideFnBlock = tracker.Update(doc, indexExpr);

		// check if lhs is an enum and can get data index from it
		auto dataIdx = ctx.userTypeInfo.GetDataIndex(indexExpr.lhsText);
		if (dataIdx.IsValid())
		{
			paramType = indexExpr.lhsText;
		}
		else if (insideFnBlock) // check if lhs is a fn parameter
		{
			const auto& fnBlock = doc.functionBlocks[tracker.index];
			assert(fnBlock.parameterNames.size() == fnBlock.parameterTypes.size());

			for (size_t i = 0; i < fnBlock.parameterNames.size(); ++i)
			{
				if (indexExpr.lhsText != fnBlock.parameterNames[i])
				{
					continue;
				}

				paramType = fnBlock.parameterTypes[i];
				dataIdx = ctx.userTypeInfo.GetDataIndex(paramType);

				break;
			}
		}

		if (!dataIdx.IsValid())
		{
			continue;
		}

		const auto& fieldNames = ctx.userTypeInfo.GetAllUserTypeFieldNames()[dataIdx];
		const auto& memberKinds = ctx.userTypeInfo.GetAllUserTypeFieldMemberKinds()[dataIdx];
		assert(fieldNames.size() == memberKinds.size());

		bool found = false;

		for (size_t i = 0; i < fieldNames.size(); ++i)
		{
			if (fieldNames[i] != indexExpr.rhsText)
			{
				continue;
			}

			if (IndexExpressionTypeMatchesMemberKind(indexExpr.type, memberKinds[i]))
			{
				found = true;
			}
			else
			{
				SourceDocumentError err{
					std::format("Type '{}' has no {} '{}'", paramType,
								IndexExpressionTypeToString(indexExpr.type), 
								indexExpr.rhsText),
					indexExpr.location
				};

				AddErrorDecoration(err, ctx.parser, ctx.textEditor, kErrorUnderlineColor);
			}

			break;
		}

		if (!found)
		{
			continue;
		}
	
		const auto coords = GetIndexExpressionRhsCoordinates(
			indexExpr, ctx.parser, ctx.textEditor);

		const auto paletteIndex = 
			(indexExpr.type == LuaSourceDocument::IndexExpressionType::Method)
				? TextDecorationUtility::UserPaletteIndex::Function
				: TextDecorationUtility::UserPaletteIndex::Property;

		[[maybe_unused]] const bool added = ctx.textEditor.AddTextColorRange(
			coords.first, coords.second, paletteIndex);

		assert(added);
	}
}

} // unnamed

void TextDecorationUtility::Update(const LuaTextEditorResourceContext& ctx)
{
	if (ctx.textEditor.IsTextChanged())
	{
		UpdateForNewText(ctx);
	}
	else if (ctx.textEditor.IsCursorPositionChanged())
	{
		UpdateForNewCursorPos(ctx);
	}
	else
	{
		UpdateForNoChange(ctx);
	}
}

void TextDecorationUtility::UpdateForNoChange(const LuaTextEditorResourceContext& ctx)
{
	if (!pendingError_)
	{
		return;
	}

	pendingError_->cursorInRegionDelta += ImGui::GetIO().DeltaTime;
	if (pendingError_->cursorInRegionDelta < kPendingErrorDelay)
	{
		return;
	}

	if (std::ranges::contains(ctx.parser.GetSourceDocument().errors, pendingError_->error))
	{
		AddErrorDecoration(pendingError_->error, ctx.parser, ctx.textEditor,
						   kErrorUnderlineColor);
	}	

	pendingError_ = {};
}

void TextDecorationUtility::UpdateForNewText(const LuaTextEditorResourceContext& ctx)
{
	Reset(ctx.textEditor);

	DrawDocumentErrorDecorations(ctx.parser, ctx.textEditor);

	DrawIndexExpressionDecorations(ctx);
}

void TextDecorationUtility::UpdateForNewCursorPos(const LuaTextEditorResourceContext& ctx)
{
	if (!pendingError_)
	{
		return;
	}

	auto cursorBytePos = TextPositionToByteOffset(ctx.parser.GetSourceString(),
												  ctx.textEditor.GetCursorPosition(),
												  ctx.textEditor.GetTabSize());

	if (cursorBytePos >= pendingError_->error.location.start &&
		cursorBytePos <= pendingError_->error.location.end)
	{
		return;
	}

	if (std::ranges::contains(ctx.parser.GetSourceDocument().errors, pendingError_->error))
	{
		AddErrorDecoration(pendingError_->error, ctx.parser, ctx.textEditor,
						   kErrorUnderlineColor);
	}	

	pendingError_ = {};
}

void TextDecorationUtility::Reset(TextEditor& textEditor)
{
	textEditor.ClearTextDecorations();
	textEditor.ClearTextTooltips();
	textEditor.ClearTextColorRanges();
	pendingError_ = {};
}

void TextDecorationUtility::DrawDocumentErrorDecorations(
	const LuaSourceParser& parser, TextEditor& textEditor)
{
	const auto& sourceDoc = parser.GetSourceDocument();

	auto cursorBytePos = TextPositionToByteOffset(parser.GetSourceString(),
												  textEditor.GetCursorPosition(),
												  textEditor.GetTabSize());

	for (const auto& err : sourceDoc.errors)
	{
		const auto errStart = ByteOffsetToCoordinates(parser.GetSourceString(),
							  err.location.start, textEditor.GetTabSize());
		const auto errEnd = ByteOffsetToCoordinates(parser.GetSourceString(),
							err.location.end, textEditor.GetTabSize());

		if (cursorBytePos >= err.location.start && cursorBytePos <= err.location.end + 1 &&
			!pendingError_ && !textEditor.IsGhostTextVisible())
		{
			// cursor inside error region, delay decoration
			pendingError_ = PendingError{err};
			continue;
		}

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