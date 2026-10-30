#pragma once

#include <string>
#include <vector>
#include <array>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <regex>
#include <string_view>
#include <span>
#include "imgui.h"
#include "../../core/TypeInfo.h"

class TextEditor
{
public:
	enum class HandledKeys : uint32_t
	{
		Z = 1 << 0,
		Backspace = 1 << 1,
		Y = 1 << 2,
		UpArrow = 1 << 3,
		DownArrow = 1 << 4,
		LeftArrow = 1 << 5,
		RightArrow = 1 << 6,
		PageUp = 1 << 7,
		PageDown = 1 << 8,
		Home = 1 << 9,
		End = 1 << 10,
		Delete = 1 << 11,
		Insert = 1 << 12,
		C = 1 << 13, 
		V = 1 << 14,
		X = 1 << 15,
		A = 1 << 16,
		Enter = 1 << 17,
		Tab = 1 << 18
	};

	enum class PaletteIndex
	{
		Default,
		Keyword,
		Number,
		String,
		CharLiteral,
		Punctuation,
		Preprocessor,
		Identifier,
		KnownIdentifier,
		PreprocIdentifier,
		Comment,
		MultiLineComment,
		Background,
		Cursor,
		Selection,
		ErrorMarker,
		Breakpoint,
		LineNumber,
		CurrentLineFill,
		CurrentLineFillInactive,
		CurrentLineEdge,
		Max
	};

	enum class SelectionMode
	{
		Normal,
		Word,
		Line
	};

	struct Breakpoint
	{
		int mLine;
		bool mEnabled;
		std::string mCondition;

		Breakpoint()
			: mLine(-1)
			, mEnabled(false)
		{}
	};

	// Represents a character coordinate from the user's point of view,
	// i. e. consider an uniform grid (assuming fixed-width font) on the
	// screen as it is rendered, and each cell has its own coordinate, starting from 0.
	// Tabs are counted as [1..mTabSize] count empty spaces, depending on
	// how many space is necessary to reach the next tab stop.
	// For example, coordinate (1, 5) represents the character 'B' in a line "\tABC", when mTabSize = 4,
	// because it is rendered as "    ABC" on the screen.
	struct Coordinates
	{
		int mLine, mColumn;
		Coordinates() : mLine(0), mColumn(0) {}
		Coordinates(int aLine, int aColumn) : mLine(aLine), mColumn(aColumn)
		{
			assert(aLine >= 0);
			assert(aColumn >= 0);
		}
		static Coordinates Invalid() { static Coordinates invalid(-1, -1); return invalid; }

		bool operator ==(const Coordinates& o) const
		{
			return
				mLine == o.mLine &&
				mColumn == o.mColumn;
		}

		bool operator !=(const Coordinates& o) const
		{
			return
				mLine != o.mLine ||
				mColumn != o.mColumn;
		}

		bool operator <(const Coordinates& o) const
		{
			if (mLine != o.mLine)
				return mLine < o.mLine;
			return mColumn < o.mColumn;
		}

		bool operator >(const Coordinates& o) const
		{
			if (mLine != o.mLine)
				return mLine > o.mLine;
			return mColumn > o.mColumn;
		}

		bool operator <=(const Coordinates& o) const
		{
			if (mLine != o.mLine)
				return mLine < o.mLine;
			return mColumn <= o.mColumn;
		}

		bool operator >=(const Coordinates& o) const
		{
			if (mLine != o.mLine)
				return mLine > o.mLine;
			return mColumn >= o.mColumn;
		}
	};

	struct Identifier
	{
		Coordinates mLocation;
		std::string mDeclaration;
	};

	typedef std::string String;
	typedef std::unordered_map<std::string, Identifier> Identifiers;
	typedef std::unordered_set<std::string> Keywords;
	typedef std::map<int, std::string> ErrorMarkers;
	typedef std::unordered_set<int> Breakpoints;
	typedef std::array<ImU32, (unsigned)PaletteIndex::Max> Palette;
	typedef uint8_t Char;

	struct Glyph
	{
		Char mChar;
		PaletteIndex mColorIndex = PaletteIndex::Default;
		bool mComment : 1;
		bool mMultiLineComment : 1;
		bool mPreprocessor : 1;

		Glyph(Char aChar, PaletteIndex aColorIndex) : mChar(aChar), mColorIndex(aColorIndex),
			mComment(false), mMultiLineComment(false), mPreprocessor(false) {}
	};

	typedef std::vector<Glyph> Line;
	typedef std::vector<Line> Lines;

	struct LanguageDefinition
	{
		typedef std::pair<std::string, PaletteIndex> TokenRegexString;
		typedef std::vector<TokenRegexString> TokenRegexStrings;
		typedef bool(*TokenizeCallback)(const char * in_begin, const char * in_end, const char *& out_begin, const char *& out_end, PaletteIndex & paletteIndex);

		std::string mName;
		Keywords mKeywords;
		Identifiers mIdentifiers;
		Identifiers mPreprocIdentifiers;
		std::string mCommentStart, mCommentEnd, mSingleLineComment;
		char mPreprocChar;
		bool mAutoIndentation;

		TokenizeCallback mTokenize;

		TokenRegexStrings mTokenRegexStrings;

		bool mCaseSensitive;

		LanguageDefinition()
			: mPreprocChar('#'), mAutoIndentation(true), mTokenize(nullptr), mCaseSensitive(true)
		{
		}

		static const LanguageDefinition& CPlusPlus();
		static const LanguageDefinition& HLSL();
		static const LanguageDefinition& GLSL();
		static const LanguageDefinition& C();
		static const LanguageDefinition& SQL();
		static const LanguageDefinition& AngelScript();
		static const LanguageDefinition& Lua();
	};

	// extensions
	struct GhostText
	{
		Coordinates position;
		std::string text;
		bool visible = false;

		bool IsVisible() const noexcept { return visible && !text.empty(); }
	};

	struct TextDecoration
	{
		enum class Type : uint8_t
		{
			None,
			WavyUnderline
		};

		Coordinates position;
		int columnSpan = 0;
		Type type = Type::None;
		ImColor color = ImColor(0, 0, 0, 255);
	};

	struct TextTooltip
	{
		struct Colors
		{
			ImColor textColor = ImColor(0, 0, 0, 0);
			ImColor bgColor = ImColor(0, 0, 0, 0);
		};

		Coordinates position;
		std::string text;
		Colors colors;

		void PushColors() const;
		void PopColors() const;
	};

	struct TextColorRange
	{
		Coordinates start;
		Coordinates end;
		PaletteIndex paletteIndex = PaletteIndex::Default;
	};

	TextEditor();
	~TextEditor();

	void SetLanguageDefinition(const LanguageDefinition& aLanguageDef);
	const LanguageDefinition& GetLanguageDefinition() const { return mLanguageDefinition; }

	const Palette& GetPalette() const { return mPaletteBase; }
	void SetPalette(const Palette& aValue);

	void SetErrorMarkers(const ErrorMarkers& aMarkers) { mErrorMarkers = aMarkers; }
	void SetBreakpoints(const Breakpoints& aMarkers) { mBreakpoints = aMarkers; }

	void Render(const char* aTitle, const ImVec2& aSize = ImVec2(), bool aBorder = false);
	void SetText(const std::string& aText);
	std::string GetText() const;

	void SetTextLines(const std::vector<std::string>& aLines);
	std::vector<std::string> GetTextLines() const;

	std::string GetSelectedText() const;
	std::string GetCurrentLineText()const;

	int GetTotalLines() const { return (int)mLines.size(); }
	bool IsOverwrite() const { return mOverwrite; }

	void SetReadOnly(bool aValue);
	bool IsReadOnly() const { return mReadOnly; }
	bool IsTextChanged() const { return mTextChanged; }
	bool IsCursorPositionChanged() const { return mCursorPositionChanged; }

	bool IsColorizerEnabled() const { return mColorizerEnabled; }
	void SetColorizerEnable(bool aValue);

	Coordinates GetCursorPosition() const { return GetActualCursorCoordinates(); }
	void SetCursorPosition(const Coordinates& aPosition);

	inline void SetHandleMouseInputs    (bool aValue){ mHandleMouseInputs    = aValue;}
	inline bool IsHandleMouseInputsEnabled() const { return mHandleMouseInputs; }

	inline void SetHandleKeyboardInputs (bool aValue){ mHandleKeyboardInputs = aValue;}
	inline bool IsHandleKeyboardInputsEnabled() const { return mHandleKeyboardInputs; }

	inline void SetImGuiChildIgnored    (bool aValue){ mIgnoreImGuiChild     = aValue;}
	inline bool IsImGuiChildIgnored() const { return mIgnoreImGuiChild; }

	inline void SetShowWhitespaces(bool aValue) { mShowWhitespaces = aValue; }
	inline bool IsShowingWhitespaces() const { return mShowWhitespaces; }

	void SetTabSize(int aValue);
	inline int GetTabSize() const { return mTabSize; }

	void InsertText(const std::string& aValue);
	void InsertText(const char* aValue);

	void MoveUp(int aAmount = 1, bool aSelect = false);
	void MoveDown(int aAmount = 1, bool aSelect = false);
	void MoveLeft(int aAmount = 1, bool aSelect = false, bool aWordMode = false);
	void MoveRight(int aAmount = 1, bool aSelect = false, bool aWordMode = false);
	void MoveTop(bool aSelect = false);
	void MoveBottom(bool aSelect = false);
	void MoveHome(bool aSelect = false);
	void MoveEnd(bool aSelect = false);

	void SetSelectionStart(const Coordinates& aPosition);
	void SetSelectionEnd(const Coordinates& aPosition);
	void SetSelection(const Coordinates& aStart, const Coordinates& aEnd, SelectionMode aMode = SelectionMode::Normal);
	void SelectWordUnderCursor();
	void SelectAll();
	bool HasSelection() const;

	void Copy();
	void Cut();
	void Paste();
	void Delete();

	bool CanUndo() const;
	bool CanRedo() const;
	void Undo(int aSteps = 1);
	void Redo(int aSteps = 1);

	static const Palette& GetDarkPalette();
	static const Palette& GetLightPalette();
	static const Palette& GetRetroBluePalette();

	// exposed
	std::string GetWordUnderCursor() const;
	Coordinates ScreenPosToCoordinates(const ImVec2& aPosition) const;
	void DeleteRange(const Coordinates& aStart, const Coordinates& aEnd);
	int GetCharacterIndex(const Coordinates& aCoordinates) const;

	// extensions
	std::pair<Coordinates, Coordinates> GetWordCoordinatesUnderCursor() const;
	ImVec2 CoordinatesToScreenPos(const Coordinates& aCoordinates) const;
	Coordinates InsertText(std::string_view aValue, PaletteIndex aColor);
	Coordinates InsertText(const char* aValue, PaletteIndex aColor);
	void SetKeyHandled(HandledKeys key, bool doHandle);
	void SetAllKeysHandled(bool doHandle);
	bool IsKeyHandled(HandledKeys key) const;
	void SetIsTextChanged(bool aValue) { mTextChanged = aValue; }
	void SetIsCursorPositionChanged(bool aValue) { mCursorPositionChanged = aValue; }
	float GetLineSpacing() const noexcept { return mLineSpacing; }

	/** @defgroup GhostText @{ */
	void SetGhostText(const Coordinates& aPosition, std::string_view aText);
	void ClearGhostText();
	const GhostText& GetGhostText() const { return mGhostText; }
	void CommitGhostText();
	void CommitGhostText(std::string_view appendText, int cursorBackPlacement = 0);
	bool IsGhostTextVisible() const noexcept { return mGhostText.IsVisible(); }
	/** @} */

	/** @defgroup TextDecoration @{ */
	bool AddTextDecoration(const Coordinates& aPos, int colSpan, TextDecoration::Type aType, ImColor aColor);
	bool AddTextDecoration(const Coordinates& aPosStart, const Coordinates& aPosEnd, TextDecoration::Type aType,
						   ImColor aColor);
	void ClearTextDecorations();
	const std::vector<TextDecoration>& GetTextDecorations() const { return mTextDecorations; }
	/** @} */

	/** @defgroup TextColorRange @{ */
	bool AddTextColorRange(const Coordinates& aStart, const Coordinates& aEnd, PaletteIndex aColor);
	template <typename T> requires std::is_enum_v<T>
	bool AddTextColorRange(Coordinates aStart, Coordinates aEnd, T aColor)
	{
		if (aStart >= aEnd)
		{
			return false;
		}

		if (mUserPaletteIndexingTypeId != TypeInfo<T>::hash32)
		{
			return false;
		}

		const auto userColorIndex = static_cast<std::underlying_type_t<T>>(aColor);
		if (userColorIndex < 0 || userColorIndex >= mUserPaletteBase.size())
		{
			return false;
		}

		mTextColorRanges.emplace_back(aStart, aEnd,
			static_cast<PaletteIndex>(userColorIndex + static_cast<int>(PaletteIndex::Max)));

		return true;
	}
	void ClearTextColorRanges() { mTextColorRanges.clear(); }
	/** @} */

	/** @defgroup TextTooltip @{ */
	bool AddTextTooltip(const Coordinates& aPos, std::string_view aText, 
						const TextTooltip::Colors& aColors = {});
	void ClearTextTooltips();
	/** @} */

	ImVec2 GetScroll() const noexcept { return mScroll; }
	ImVec2 GetWindowMin() const noexcept { return mWindowMin; }
	ImVec2 GetWindowMax() const noexcept { return mWindowMax; }

	bool MouseInsideWindow() const;

	/** @defgroup UserPalette @{ */
	template <typename T> requires std::is_enum_v<T>
	void SetUserPalette(const Palette& aPalette)
	{
		mUserPaletteBase = aPalette;
		mUserPaletteIndexingTypeId = TypeInfo<T>::hash32;
	}
	const Palette& GetUserPalette() const { return mUserPaletteBase; }
	/** @} */

private:
	typedef std::vector<std::pair<std::regex, PaletteIndex>> RegexList;

	struct EditorState
	{
		Coordinates mSelectionStart;
		Coordinates mSelectionEnd;
		Coordinates mCursorPosition;
	};

	class UndoRecord
	{
	public:
		UndoRecord() {}
		~UndoRecord() {}

		UndoRecord(
			const std::string& aAdded,
			const TextEditor::Coordinates aAddedStart,
			const TextEditor::Coordinates aAddedEnd,

			const std::string& aRemoved,
			const TextEditor::Coordinates aRemovedStart,
			const TextEditor::Coordinates aRemovedEnd,

			TextEditor::EditorState& aBefore,
			TextEditor::EditorState& aAfter);

		void Undo(TextEditor* aEditor);
		void Redo(TextEditor* aEditor);

		std::string mAdded;
		Coordinates mAddedStart;
		Coordinates mAddedEnd;

		std::string mRemoved;
		Coordinates mRemovedStart;
		Coordinates mRemovedEnd;

		EditorState mBefore;
		EditorState mAfter;
	};

	// extension
	struct TextDecorationRenderData
	{
		struct Entry
		{
			int glyphStart = 0;
			int glyphEnd = 0;
			float startX = -1.0f;
			float endX = -1.0f;
			TextDecoration::Type type = TextDecoration::Type::None;
			ImU32 color = 0;
		};

		std::vector<Entry> entries;

		void InitializeForNewLine(const Line& line, int lineNo, const TextEditor& editor);
		// Call before processing the glyph at glyphIdx (and once with line.size() at line end).
		bool IsBoundary(int glyphIdx) const;
		void Update(float currentX, int glyphIdx);
		void DrawDecorations(const ImVec2& textScreenPos);
		void Reset();

	};

	typedef std::vector<UndoRecord> UndoBuffer;

	void ProcessInputs();
	void Colorize(int aFromLine = 0, int aCount = -1);
	void ColorizeRange(int aFromLine = 0, int aToLine = 0);
	void ColorizeInternal();
	float TextDistanceToLineStart(const Coordinates& aFrom) const;
	void EnsureCursorVisible();
	int GetPageSize() const;
	std::string GetText(const Coordinates& aStart, const Coordinates& aEnd) const;
	Coordinates GetActualCursorCoordinates() const;
	Coordinates SanitizeCoordinates(const Coordinates& aValue) const;
	void Advance(Coordinates& aCoordinates) const;
	int InsertTextAt(Coordinates& aWhere, const char* aValue, PaletteIndex aColor = PaletteIndex::Default);
	void AddUndo(UndoRecord& aValue);
	Coordinates FindWordStart(const Coordinates& aFrom) const;
	Coordinates FindWordEnd(const Coordinates& aFrom) const;
	Coordinates FindNextWord(const Coordinates& aFrom) const;	
	int GetCharacterColumn(int aLine, int aIndex) const;
	int GetLineCharacterCount(int aLine) const;
	int GetLineMaxColumn(int aLine) const;
	bool IsOnWordBoundary(const Coordinates& aAt) const;
	void RemoveLine(int aStart, int aEnd);
	void RemoveLine(int aIndex);
	Line& InsertLine(int aIndex);
	void EnterCharacter(ImWchar aChar, bool aShift);
	void Backspace();
	void DeleteSelection();
	std::string GetWordAt(const Coordinates& aCoords) const;
	ImU32 GetGlyphColor(const Glyph& aGlyph) const;

	// extensions
	void ApplyTextColorRanges();

	std::pair<Coordinates, Coordinates> CalculateNormalSelectionCoordinates(const Coordinates& aStart,
																			const Coordinates& aEnd) const;
	void RenderColorizedText(const Line& line, ImVec2 textScreenPos, int lineNo, 
							 int& columnNo, float spaceSize);
	std::pair<Coordinates, Coordinates> GetWordCoordinatesAt(const Coordinates& aCoords) const;

	uint32_t mHandledKeys = 0xFFFFFFFF;
	GhostText mGhostText;
	std::vector<TextDecoration> mTextDecorations;
	TextDecorationRenderData mTextDecorationRenderData;
	std::vector<TextTooltip> mTextTooltips;
	ImVec2 mScreenOrigin = {};
	ImVec2 mScroll = {};
	ImVec2 mWindowMin = {};
	ImVec2 mWindowMax = {};
	Palette mUserPaletteBase;
	Palette mUserPalette;
	uint32_t mUserPaletteIndexingTypeId = 0;
	std::vector<TextColorRange> mTextColorRanges;

	void HandleKeyboardInputs();
	void HandleMouseInputs();
	void Render();

	float mLineSpacing;
	Lines mLines;
	EditorState mState;
	UndoBuffer mUndoBuffer;
	int mUndoIndex;

	int mTabSize;
	bool mOverwrite;
	bool mReadOnly;
	bool mWithinRender;
	bool mScrollToCursor;
	bool mScrollToTop;
	bool mTextChanged;
	bool mColorizerEnabled;
	float mTextStart;                   // position (in pixels) where a code line starts relative to the left of the TextEditor.
	int  mLeftMargin;
	bool mCursorPositionChanged;
	int mColorRangeMin, mColorRangeMax;
	SelectionMode mSelectionMode;
	bool mHandleKeyboardInputs;
	bool mHandleMouseInputs;
	bool mIgnoreImGuiChild;
	bool mShowWhitespaces;

	Palette mPaletteBase;
	Palette mPalette;
	LanguageDefinition mLanguageDefinition;
	RegexList mRegexList;

	bool mCheckComments;
	Breakpoints mBreakpoints;
	ErrorMarkers mErrorMarkers;
	ImVec2 mCharAdvance;
	Coordinates mInteractiveStart, mInteractiveEnd;
	std::string mLineBuffer;
	uint64_t mStartTime;

	float mLastClick;
};
