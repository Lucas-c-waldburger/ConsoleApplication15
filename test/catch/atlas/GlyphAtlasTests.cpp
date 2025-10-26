#include "../CatchMain.cpp"
#include "../../../atlas/NewGlyphAtlas.h"
#include "../../../file/FilePathUtility.h"
#include "../../../sdl/SDLite.h"

namespace {

constexpr std::string_view kFontReg = "GoNotoKurrent-Regular.ttf";
constexpr std::string_view kFontBold = "GoNotoKurrent-Bold.ttf";

} // unnamed

TEST_CASE("Glyph Atlas tests", "[atlas]")
{
	Logger::StartSession();
	auto status = SDLite::Start();
	REQUIRE(status.Good());

	auto pathResult = ResourcePath::Font(kFontReg);
	REQUIRE_RESULT(pathResult);

	FontDescriptor descriptorReg{
		.fontName = "GoNotoCurrent-Regular",
		.filepath = pathResult.GetValue(),
		.fontSize = 24
	};

	auto loadResult = NewGlyphAtlas::Create(SDLite::Renderer(), std::move(descriptorReg));
	REQUIRE_RESULT(loadResult);

	auto& glyphAtlas = loadResult.GetValue();
	CHECK(glyphAtlas.IsLoaded());
	CHECK(glyphAtlas.GetHandle().IsValid());
	CHECK(glyphAtlas.GetSourceTexture() != nullptr);
	
	// check descriptor
	const auto& atlasDesc = glyphAtlas.GetFontDescriptor();
	CHECK(atlasDesc.fontName == "GoNotoCurrent-Regular");
	CHECK(atlasDesc.filepath == pathResult.GetValue());
	CHECK(atlasDesc.fontSize == 24);
	CHECK(atlasDesc.fontHeight > 0); // font height got set internally

	// check retrieving single glyph
	auto glyphA = glyphAtlas.GetGlyph('A');
	CHECK(glyphA.character == 'A');
	CHECK(glyphA.plot.rect.w > 0);
	CHECK(glyphA.plot.rect.h > 0);
	CHECK(glyphA.plot.rotation == 0.0f);
	CHECK(glyphA.advance > 0);

	// check retrieving glyphs for string
	constexpr std::string_view text = "U wot m8?";
	auto glyphsForText = glyphAtlas.GetGlyphsForString(text);
	REQUIRE(glyphsForText.size() == text.size());

	for (size_t i = 0; i < text.size(); i++)
	{
		CHECK(glyphsForText[i].character == text[i]);
		CHECK(glyphsForText[i].plot.rect.w > 0);
		CHECK(glyphsForText[i].plot.rect.h > 0);
		CHECK(glyphsForText[i].plot.rotation == 0.0f);
		CHECK(glyphsForText[i].advance > 0);
	}

	SDLite::Exit();
}


