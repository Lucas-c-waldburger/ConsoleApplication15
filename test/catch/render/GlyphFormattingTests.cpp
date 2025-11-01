#include "../CatchUtils.h"
#include "../../../systems/GlyphFormattingSystem.h"
#include "../../../sdl/SDLite.h"
#include "../../../ecs/Ecs.h"
#include "../../../file/FilePathUtility.h"
#include "../../../atlas/NewGlyphAtlas.h"

TEST_CASE("Glyph Formatting Tests", "[rendering]")
{
	Logger::StartSession();
	auto status = SDLite::Start();
	REQUIRE(status.Good());

	auto pathResult = ResourcePath::Font("GoNotoKurrent-Bold.ttf");

	REQUIRE_RESULT(pathResult); 
	FontDescriptor fontDesc{
		.fontName = "GoNotoCurrent-Bold",
		.filepath = pathResult.GetValue(),
		.fontSize = 24
	};

	auto glyphAtlasResult = NewGlyphAtlas::Create(SDLite::Renderer(), std::move(fontDesc));
	REQUIRE_RESULT(glyphAtlasResult);
	CHECK(glyphAtlasResult.GetValue().IsLoaded());

	const auto glyphAtlasHandle = glyphAtlasResult.GetValue().GetHandle();
	CHECK(glyphAtlasHandle.IsValid());

	auto entity = ECS::CreateEntity();
	REQUIRE(entity.IsValid());

	auto& renderable = entity.AddComponent<NewRenderable>();

	const std::string text = "check it out, it's some text";

	renderable.sourceAtlas = glyphAtlasHandle;
	renderable.profile.mods.color = { 0, 0, 0 };
	renderable.renderData = NewTextRenderable{
		.text = text,
		.formatting = TextFormatting{
			.bounds = { 200, 100 },
			.align = TextAlign::Left
		}
	};

	auto& tf = entity.AddComponent(Transform{ 
		.position = { 200.0f, 200.0f},
		.rotation = 45.0f
	});

	GlyphCacheHandler formattingSystem{};

	formattingSystem.Update(glyphAtlasResult.GetValue());

	REQUIRE(entity.HasComponent<TextRenderableGlyphCache>());

	auto& glyphCache = entity.GetComponent<TextRenderableGlyphCache>();

	auto glyphsFromAtlas = glyphAtlasResult.GetValue().GetGlyphsForString(text);
	
	REQUIRE(glyphCache.cache.size() == text.size());
	REQUIRE(glyphCache.cache.size() == glyphsFromAtlas.size());

	for (size_t i = 0; i < glyphCache.cache.size(); i++)
	{
		CHECK(glyphCache.cache[i].glyph.character == text[i]);
		CHECK(glyphCache.cache[i].glyph == glyphsFromAtlas[i]);

		CHECK(glyphCache.cache[i].destRect.w > 0);
		CHECK(glyphCache.cache[i].destRect.h > 0);
	}



	SDLite::Exit();
}