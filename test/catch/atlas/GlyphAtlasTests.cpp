#include "../CatchUtils.h"
#include "../../../atlas/GlyphAtlasCollection.h"
#include "../../../file/FilePathUtility.h"
#include "../../../sdl/SDLite.h"

namespace {

constexpr std::string_view kFontReg = "GoNotoKurrent-Regular.ttf";
constexpr std::string_view kFontBold = "GoNotoKurrent-Bold.ttf";

struct MockTextureObserver : public TextureObserver
{
	std::unordered_map<TextureAtlasID, SDL_Texture*> atlasIdToTexture;
	SignalToken token;

	void Connect(FontAtlas& collection)
	{
		token = collection.ConnectTextureObserver(GetTextureObserverPassKey(),
			[this](TextureAtlasID id, SDL_Texture* tx) {
				atlasIdToTexture[id] = tx;
			});
	}
};

} // unnamed

//TEST_CASE("Font Atlas tests", "[atlas]")
//{
//	Logger::StartSession();
//	auto status = SDLite::Start();
//	REQUIRE(status.Good());
//
//	auto pathResult = ResourcePath::Font(kFontReg);
//	REQUIRE_RESULT(pathResult);
//
//	FontDescriptor descriptorReg{
//		.fontName = "GoNotoKurrent-Regular",
//		.filepath = pathResult.GetValue(),
//		.fontSize = 24
//	};
//	
//	FontAtlas fontAtlas{};
//
//	auto loadResult = fontAtlas.LoadFont(SDLite::Renderer(), std::move(descriptorReg));
//	REQUIRE_RESULT(loadResult);
//
//	const auto& font = fontAtlas.GetFont("GoNotoKurrent-Regular");
//
//	CHECK(font.IsLoaded());
//	CHECK(font.GetSourceTexture() != nullptr);
//	
//	// check font Info
//	auto fontInfo = fontAtlas.GetFontInfo("GoNotoKurrent-Regular");
//	REQUIRE(fontInfo.has_value());
//
//	auto [atlasId, name, path, size, height] = *fontInfo;
//
//	CHECK(atlasId == font.GetAtlasID());
//	CHECK(name == "GoNotoKurrent-Regular");
//	CHECK(path == pathResult.GetValue());
//	CHECK(size == 24);
//	CHECK(height > 0); // font height got set internally
//
//	// check retrieving single glyph
//	auto glyphA = font.GetGlyph('A');
//	CHECK(glyphA.character == 'A');
//	CHECK(glyphA.plot.rect.w > 0);
//	CHECK(glyphA.plot.rect.h > 0);
//	CHECK(glyphA.plot.rotation == 0.0f);
//	CHECK(glyphA.advance > 0);
//
//	// check retrieving glyphs for string
//	constexpr std::string_view text = "U wot m8?";
//	auto glyphsForText = glyphAtlas.GetGlyphsForString(text);
//	REQUIRE(glyphsForText.size() == text.size());
//
//	for (size_t i = 0; i < text.size(); i++)
//	{
//		CHECK(glyphsForText[i].character == text[i]);
//		CHECK(glyphsForText[i].plot.rect.w > 0);
//		CHECK(glyphsForText[i].plot.rect.h > 0);
//		CHECK(glyphsForText[i].plot.rotation == 0.0f);
//		CHECK(glyphsForText[i].advance > 0);
//	}
//
//	SDLite::Exit();
//}

TEST_CASE("Font Atlas Tests", "[atlas]")
{
	Logger::StartSession();
	auto status = SDLite::Start();
	REQUIRE(status.Good());

	auto regPathResult = ResourcePath::Font(kFontReg);
	REQUIRE_RESULT(regPathResult);

	FontDescriptor descriptorReg{
		.fontName = "GoNotoKurrent-Regular",
		.filepath = regPathResult.GetValue(),
		.fontSize = 24
	};
	auto descriptorRegCopy = descriptorReg;

	auto boldPathResult = ResourcePath::Font(kFontBold);
	REQUIRE_RESULT(boldPathResult);

	FontDescriptor descriptorBold{
		.filepath = boldPathResult.GetValue(),
		.fontSize = 36
	};
	auto descriptorBoldCopy = descriptorBold;

	SECTION("Load single font")
	{
		FontAtlas fontAtlas{};
		CHECK(fontAtlas.GetTextureCount() == 0);

		auto loadResult = fontAtlas.LoadFont(
			SDLite::Renderer(), std::move(descriptorReg));
		REQUIRE_RESULT(loadResult);

		CHECK(fontAtlas.GetTextureCount() == 1);
		REQUIRE(fontAtlas.HasFont("GoNotoKurrent-Regular"));

		auto info = fontAtlas.GetFontInfo("GoNotoKurrent-Regular");
		REQUIRE(info.has_value());

		const auto [atlasId, name, path, size, height] = *info;
		CHECK(atlasId != kInvalidTextureAtlasID);
		CHECK(name == "GoNotoKurrent-Regular");
		CHECK(path == regPathResult.GetValue());
		CHECK(size == 24);
		CHECK(height > 0);

		const auto& regFont = fontAtlas.GetFont("GoNotoKurrent-Regular");
		CHECK(regFont.IsLoaded());
		CHECK(regFont.GetSourceTexture() != nullptr);

		// check retrieving single glyph
		auto glyphA = regFont.GetGlyph('A');
		CHECK(glyphA.character == 'A');
		CHECK(glyphA.plot.rect.w > 0);
		CHECK(glyphA.plot.rect.h > 0);
		CHECK(glyphA.plot.rotation == 0.0f);
		CHECK(glyphA.advance > 0);

		// check retrieving glyphs for string
		constexpr std::string_view text = "U wot m8?";
		auto glyphsForText = regFont.GetGlyphsForString(text);
		REQUIRE(glyphsForText.size() == text.size());

		for (size_t i = 0; i < text.size(); i++)
		{
			CHECK(glyphsForText[i].character == text[i]);
			CHECK(glyphsForText[i].plot.rect.w > 0);
			CHECK(glyphsForText[i].plot.rect.h > 0);
			CHECK(glyphsForText[i].plot.rotation == 0.0f);
			CHECK(glyphsForText[i].advance > 0);
		}

		// text writer
		auto writer = fontAtlas.GetTextWriter("GoNotoKurrent-Regular");
		CHECK(writer.resourceHandle.IsValid());
		CHECK(writer.resourceHandle.GetAtlasID() == regFont.GetAtlasID());
		CHECK(fontAtlas.IsTextWriterValid(writer));
	}

	SECTION("Load multiple fonts")
	{
		FontAtlas fontAtlas{};
		CHECK(fontAtlas.GetTextureCount() == 0);
	
		FontDescriptors descriptors{ descriptorRegCopy, descriptorBoldCopy };

		auto loadResult = fontAtlas.LoadFonts(
			SDLite::Renderer(), std::move(descriptors));
		REQUIRE_RESULT(loadResult);

		CHECK(fontAtlas.GetTextureCount() == 2);
		REQUIRE(fontAtlas.HasFont("GoNotoKurrent-Regular"));
		REQUIRE(fontAtlas.HasFont("GoNotoKurrent-Bold")); // name auto-assigned from stem

		auto regInfo = fontAtlas.GetFontInfo("GoNotoKurrent-Regular");
		REQUIRE(regInfo.has_value());

		const auto [regAtlasId, regName, regPath, regSize, regHeight] = *regInfo;
		CHECK(regAtlasId != kInvalidTextureAtlasID);
		CHECK(regName == "GoNotoKurrent-Regular");
		CHECK(regPath == regPathResult.GetValue());
		CHECK(regSize == 24);
		CHECK(regHeight > 0);

		auto boldInfo = fontAtlas.GetFontInfo("GoNotoKurrent-Bold");
		REQUIRE(boldInfo.has_value());

		const auto [boldAtlasId, boldName, boldPath, boldSize, boldHeight] = *boldInfo;
		CHECK(boldAtlasId != kInvalidTextureAtlasID);
		CHECK(boldName == "GoNotoKurrent-Bold");
		CHECK(boldPath == boldPathResult.GetValue());
		CHECK(boldSize == 36);
		CHECK(boldHeight > 0);

		const auto& regFont = fontAtlas.GetFont("GoNotoKurrent-Regular");
		CHECK(regFont.IsLoaded());
		CHECK(regFont.GetAtlasID() == regAtlasId);
		CHECK(regFont.GetSourceTexture() != nullptr);

		const auto& boldFont = fontAtlas.GetFont("GoNotoKurrent-Bold");
		CHECK(boldFont.IsLoaded());
		CHECK(boldFont.GetAtlasID() == boldAtlasId);
		CHECK(boldFont.GetSourceTexture() != nullptr);

		// check retrieving single glyph
		auto glyphA = regFont.GetGlyph('A');
		CHECK(glyphA.character == 'A');
		CHECK(glyphA.plot.rect.w > 0);
		CHECK(glyphA.plot.rect.h > 0);
		CHECK(glyphA.plot.rotation == 0.0f);
		CHECK(glyphA.advance > 0);

		auto glyphB = boldFont.GetGlyph('?');
		CHECK(glyphB.character == '?');
		CHECK(glyphB.plot.rect.w > 0);
		CHECK(glyphB.plot.rect.h > 0);
		CHECK(glyphB.plot.rotation == 0.0f);
		CHECK(glyphB.advance > 0);

		// check retrieving glyphs for string
		constexpr std::string_view regText = "U wot m8?";
		auto regGlyphsForText = regFont.GetGlyphsForString(regText);
		REQUIRE(regGlyphsForText.size() == regText.size());

		for (size_t i = 0; i < regText.size(); i++)
		{
			CHECK(regGlyphsForText[i].character == regText[i]);
			CHECK(regGlyphsForText[i].plot.rect.w > 0);
			CHECK(regGlyphsForText[i].plot.rect.h > 0);
			CHECK(regGlyphsForText[i].plot.rotation == 0.0f);
			CHECK(regGlyphsForText[i].advance > 0);
		}

		constexpr std::string_view boldText = "(#123-+)";
		auto boldGlyphsForText = regFont.GetGlyphsForString(boldText);
		REQUIRE(boldGlyphsForText.size() == boldText.size());

		for (size_t i = 0; i < boldText.size(); i++)
		{
			CHECK(boldGlyphsForText[i].character == boldText[i]);
			CHECK(boldGlyphsForText[i].plot.rect.w > 0);
			CHECK(boldGlyphsForText[i].plot.rect.h > 0);
			CHECK(boldGlyphsForText[i].plot.rotation == 0.0f);
			CHECK(boldGlyphsForText[i].advance > 0);
		}
	}

	SECTION("Texture Created Notification")
	{
		FontAtlas fontAtlas{};
		CHECK(fontAtlas.GetTextureCount() == 0);

		MockTextureObserver mockObserver{};
		mockObserver.Connect(fontAtlas);

		CHECK(mockObserver.token.IsConnected());
		CHECK(mockObserver.atlasIdToTexture.empty());

		auto loadResult = fontAtlas.LoadFont(
			SDLite::Renderer(), std::move(descriptorBoldCopy));
		REQUIRE_RESULT(loadResult);

		CHECK(fontAtlas.GetTextureCount() == 1);
		REQUIRE(fontAtlas.HasFont("GoNotoKurrent-Bold"));

		const auto& regFont = fontAtlas.GetFont("GoNotoKurrent-Bold");
		CHECK(regFont.IsLoaded());
		CHECK(regFont.GetAtlasID() != kInvalidTextureAtlasID);
		CHECK(regFont.GetSourceTexture() != nullptr);

		CHECK(mockObserver.token.IsConnected());
		REQUIRE(mockObserver.atlasIdToTexture.size() == 1);
		
		const auto& [atlasId, texture] = *mockObserver.atlasIdToTexture.begin();
		CHECK(atlasId == regFont.GetAtlasID());
		CHECK(texture == regFont.GetSourceTexture());
	}

	SDLite::Exit();
}

