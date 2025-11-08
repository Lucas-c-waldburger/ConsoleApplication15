#include "../CatchUtils.h"
#include "../../../sdl/SDLite.h"
#include "../../../ecs/Ecs.h"
#include "../../../file/FilePathUtility.h"
#include "../../../atlas/NewGlyphAtlas.h"
#include "../../../render/GlyphCacheHandler.h"
#include "../test_utils/ImageUtilities.h"

namespace {

void RenderTextGlyphs(std::string_view fileName, const TextRenderableGlyphCache& glyphCache, 
					  const TextRenderableComponent& textRenderable, 
					  const Transform& tf, SDL_Texture* srcTexture)
{
	REQUIRE(srcTexture != nullptr);

	auto [r, g, b] = textRenderable.profile.mods.color;
	SDL_SetTextureColorMod(srcTexture, r, g, b);
	SDL_SetTextureAlphaMod(srcTexture, textRenderable.profile.mods.alpha);
	SDL_SetTextureBlendMode(srcTexture, SDL_BLENDMODE_BLEND);

	for (const auto& [glyph, destRect, rotCenter] : glyphCache.cache)
	{
		if (glyph == NewGlyphAtlas::kNewlineGlyph)
		{
			continue;
		}
		SDL_RenderCopyEx(SDLite::Renderer(), srcTexture, &glyph.plot.rect, &destRect, 
			static_cast<double>(tf.rotation + glyph.plot.rotation), &rotCenter,
			textRenderable.profile.flip);
	}

	SDLite::Renderer().Show();

	auto imgWriteResult = test::WriteFrameBufferToPNG(SDLite::Renderer(), fileName);
	REQUIRE_RESULT(imgWriteResult);
}


} // unnamed

TEST_CASE("GlyphCacheHandler Tests", "[rendering]")
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

	auto& glyphAtlas = glyphAtlasResult.GetValue();
	CHECK(glyphAtlas.GetHandle().IsValid());

	TextRenderableComponent textRenderable{
		.writer = glyphAtlas.GetTextWriter(),
		.formatting = {
			.bounds = { 500, 200 },
			.scaleToBounds = false
		}
	};
	CHECK(textRenderable.writer.sourceAtlas == glyphAtlas.GetHandle());

	textRenderable.writer.text = "Rusty shackleford";

	auto glyphsFromAtlas = glyphAtlas.GetGlyphsForString(textRenderable.writer.text);

	Transform transform{
		.position = { SDLite::kFWindowCenter.x - 150, SDLite::kFWindowCenter.y }
	};

	TextRenderableGlyphCache glyphCache{};

	GlyphCacheHandler::UpdateGlyphCache(glyphAtlas, textRenderable, 
										glyphCache, transform);

	auto& [glyphs, ctx] = glyphCache;

	// Context updated correctly
	CHECK(ctx.sourceAtlas == textRenderable.writer.sourceAtlas);
	CHECK(ctx.transform == transform);
	CHECK(ctx.formatting == textRenderable.formatting);

	// Glyphs match expected
	REQUIRE(textRenderable.writer.text.size() == glyphs.size());
	for (size_t i = 0; i < glyphs.size(); i++)
	{
		CHECK(glyphs[i].glyph.character == textRenderable.writer.text[i]);
		CHECK(glyphs[i].glyph == glyphsFromAtlas[i]);

		CHECK(glyphs[i].destRect.w > 0);
		CHECK(glyphs[i].destRect.h > 0);
	}

	auto renderOriginalTransparent = 
		[srcTexture = glyphAtlas.GetSourceTexture(), cacheCopy = glyphCache,
		 color = textRenderable.profile.mods.color, 
		tfRot = transform.rotation, flip = textRenderable.profile.flip] {

		SDL_PumpEvents();

		SDLite::Renderer().Clear(SDLite::kColorWhite);

		auto [r, g, b] = color;
		SDL_SetTextureColorMod(srcTexture, r, g, b);
		SDL_SetTextureAlphaMod(srcTexture, 90);
		SDL_SetTextureBlendMode(srcTexture, SDL_BLENDMODE_BLEND);

		for (const auto& [glyph, destRect, rotCenter] : cacheCopy.cache)
		{
			if (glyph == NewGlyphAtlas::kNewlineGlyph)
			{
				continue;
			}
			SDL_RenderCopyEx(SDLite::Renderer(), srcTexture, &glyph.plot.rect, &destRect,
				static_cast<double>(tfRot + glyph.plot.rotation), &rotCenter, flip);
		}
	};

	SECTION("No changes")
	{
		SDL_PumpEvents();

		SDLite::Renderer().Clear(SDLite::kColorWhite);

		RenderTextGlyphs("GlyphCacheHandler_no_changes.png",
			glyphCache, textRenderable, transform, glyphAtlas.GetSourceTexture());

		SDLite::Exit();
	}

	SECTION("Change transform position")
	{
		transform.position.x += 50.0f;
		transform.position.y -= 50.0f;

		auto [oldGlyphs, oldCtx] = glyphCache;

		GlyphCacheHandler::UpdateGlyphCache(glyphAtlas, textRenderable, 
											glyphCache, transform);

		auto& [newGlyphs, newCtx] = glyphCache;
		CHECK(newCtx.transform == transform);
		CHECK(newCtx.formatting == textRenderable.formatting);
		CHECK(newCtx.sourceAtlas == textRenderable.writer.sourceAtlas);
		CHECK(newCtx.offset == textRenderable.profile.offset);

		REQUIRE(newGlyphs.size() == oldGlyphs.size());
		for (size_t i = 0; i < newGlyphs.size(); i++)
		{
			CHECK(oldGlyphs[i].glyph == newGlyphs[i].glyph);
			CHECK(oldGlyphs[i].rotationCenter == newGlyphs[i].rotationCenter);

			CHECK(oldGlyphs[i].destRect.w == newGlyphs[i].destRect.w);
			CHECK(oldGlyphs[i].destRect.h == newGlyphs[i].destRect.h);

			CHECK(oldGlyphs[i].destRect.x != newGlyphs[i].destRect.x);
			CHECK(oldGlyphs[i].destRect.y != newGlyphs[i].destRect.y);
		}

		renderOriginalTransparent();
		RenderTextGlyphs("GlyphCacheHandler_change_transform_pos.png",
			glyphCache, textRenderable, transform, glyphAtlas.GetSourceTexture());

		SDLite::Exit();
	}

	SECTION("Change offset position")
	{
		textRenderable.profile.offset.x -= 44.0f;
		textRenderable.profile.offset.y += 23.0f;

		auto [oldGlyphs, oldCtx] = glyphCache;

		GlyphCacheHandler::UpdateGlyphCache(glyphAtlas, textRenderable,
											glyphCache, transform);

		auto& [newGlyphs, newCtx] = glyphCache;
		CHECK(newCtx.transform == transform);
		CHECK(newCtx.formatting == textRenderable.formatting);
		CHECK(newCtx.sourceAtlas == textRenderable.writer.sourceAtlas);
		CHECK(newCtx.offset == textRenderable.profile.offset);

		REQUIRE(newGlyphs.size() == oldGlyphs.size());
		for (size_t i = 0; i < newGlyphs.size(); i++)
		{
			CHECK(oldGlyphs[i].glyph == newGlyphs[i].glyph);
			CHECK(oldGlyphs[i].rotationCenter == newGlyphs[i].rotationCenter);

			CHECK(oldGlyphs[i].destRect.w == newGlyphs[i].destRect.w);
			CHECK(oldGlyphs[i].destRect.h == newGlyphs[i].destRect.h);

			CHECK(oldGlyphs[i].destRect.x != newGlyphs[i].destRect.x);
			CHECK(oldGlyphs[i].destRect.y != newGlyphs[i].destRect.y);
		}

		renderOriginalTransparent();
		RenderTextGlyphs("GlyphCacheHandler_change_offset_pos.png",
			glyphCache, textRenderable, transform, glyphAtlas.GetSourceTexture());

		SDLite::Exit();
	}

	SECTION("Change scale")
	{
		transform.scale.x = 2.0f;
		transform.scale.y = 2.0f;

		auto [oldGlyphs, oldCtx] = glyphCache;

		GlyphCacheHandler::UpdateGlyphCache(glyphAtlas, textRenderable,
			glyphCache, transform);

		auto& [newGlyphs, newCtx] = glyphCache;
		CHECK(newCtx.transform == transform);
		CHECK(newCtx.formatting == textRenderable.formatting);
		CHECK(newCtx.sourceAtlas == textRenderable.writer.sourceAtlas);
		CHECK(newCtx.offset == textRenderable.profile.offset);

		REQUIRE(newGlyphs.size() == oldGlyphs.size());
		for (size_t i = 0; i < newGlyphs.size(); i++)
		{
			CHECK(oldGlyphs[i].glyph == newGlyphs[i].glyph);
			//CHECK(oldGlyphs[i].rotationCenter != newGlyphs[i].rotationCenter);

			CHECK(oldGlyphs[i].destRect.w < newGlyphs[i].destRect.w);
			CHECK(oldGlyphs[i].destRect.h < newGlyphs[i].destRect.h);

			CHECK(oldGlyphs[i].destRect.x != newGlyphs[i].destRect.x);
			CHECK(oldGlyphs[i].destRect.y != newGlyphs[i].destRect.y);
		}

		renderOriginalTransparent();
		RenderTextGlyphs("GlyphCacheHandler_change_scale.png",
			glyphCache, textRenderable, transform, glyphAtlas.GetSourceTexture());

		SDLite::Exit();
	}

	SECTION("Change rotation")
	{
		transform.rotation += 30.0f;

		auto [oldGlyphs, oldCtx] = glyphCache;

		GlyphCacheHandler::UpdateGlyphCache(glyphAtlas, textRenderable,
											glyphCache, transform);

		auto& [newGlyphs, newCtx] = glyphCache;
		CHECK(newCtx.transform == transform);
		CHECK(newCtx.formatting == textRenderable.formatting);
		CHECK(newCtx.sourceAtlas == textRenderable.writer.sourceAtlas);
		CHECK(newCtx.offset == textRenderable.profile.offset);

		REQUIRE(newGlyphs.size() == oldGlyphs.size());
		for (size_t i = 0; i < newGlyphs.size(); i++)
		{
			CHECK(oldGlyphs[i].glyph == newGlyphs[i].glyph);
			//CHECK(oldGlyphs[i].rotationCenter != newGlyphs[i].rotationCenter);

			CHECK(oldGlyphs[i].destRect.w == newGlyphs[i].destRect.w);
			CHECK(oldGlyphs[i].destRect.h == newGlyphs[i].destRect.h);

			CHECK(oldGlyphs[i].destRect.x != newGlyphs[i].destRect.x);
			CHECK(oldGlyphs[i].destRect.y != newGlyphs[i].destRect.y);
		}

		renderOriginalTransparent();
		RenderTextGlyphs("GlyphCacheHandler_change_rotation.png",
			glyphCache, textRenderable, transform, glyphAtlas.GetSourceTexture());

		SDLite::Exit();
	}

	SECTION("Change text")
	{
		textRenderable.writer.text = "Dale\nGribble";
		static constexpr size_t kNewlineIndex = 4;

		auto [oldGlyphs, oldCtx] = glyphCache;

		GlyphCacheHandler::UpdateGlyphCache(glyphAtlas, textRenderable,
			glyphCache, transform);

		auto& [newGlyphs, newCtx] = glyphCache;
		CHECK(newCtx.transform == transform);
		CHECK(newCtx.formatting == textRenderable.formatting);
		CHECK(newCtx.sourceAtlas == textRenderable.writer.sourceAtlas);
		CHECK(newCtx.offset == textRenderable.profile.offset);

		auto newGlyphsFromAtlas = glyphAtlas.GetGlyphsForString(textRenderable.writer.text);

		REQUIRE(newGlyphs.size() != oldGlyphs.size());
		REQUIRE(newGlyphs.size() == newGlyphsFromAtlas.size());
		REQUIRE(newGlyphs.size() == textRenderable.writer.text.size());
		for (size_t i = 0; i < newGlyphs.size(); i++)
		{
			CHECK(newGlyphs[i].glyph == newGlyphsFromAtlas[i]);
			CHECK(newGlyphs[i].glyph.character == textRenderable.writer.text[i]);

			if (newGlyphs[i].glyph != NewGlyphAtlas::kNewlineGlyph)
			{
				CHECK(newGlyphs[i].destRect.w > 0);
				CHECK(newGlyphs[i].destRect.h > 0);
			}
		}

		renderOriginalTransparent();
		RenderTextGlyphs("GlyphCacheHandler_change_text.png",
			glyphCache, textRenderable, transform, glyphAtlas.GetSourceTexture());

		SDLite::Exit();
	}

	SECTION("Change formatting")
	{
		static constexpr std::array kOtherTextAligns = {
			std::make_pair(TextAlign::Right, "right"),
			std::make_pair(TextAlign::Center, "center")
		};

		for (const auto& [align, alignStr] : kOtherTextAligns)
		{
			textRenderable.formatting.align = align;

			auto [oldGlyphs, oldCtx] = glyphCache;

			GlyphCacheHandler::UpdateGlyphCache(glyphAtlas, textRenderable,
				glyphCache, transform);

			auto& [newGlyphs, newCtx] = glyphCache;
			CHECK(newCtx.transform == transform);
			CHECK(newCtx.formatting == textRenderable.formatting);
			CHECK(newCtx.sourceAtlas == textRenderable.writer.sourceAtlas);
			CHECK(newCtx.offset == textRenderable.profile.offset);

			REQUIRE(newGlyphs.size() == oldGlyphs.size());
			for (size_t i = 0; i < newGlyphs.size(); i++)
			{
				CHECK(oldGlyphs[i].glyph == newGlyphs[i].glyph);
				//CHECK(oldGlyphs[i].rotationCenter != newGlyphs[i].rotationCenter);

				CHECK(oldGlyphs[i].destRect.w == newGlyphs[i].destRect.w);
				CHECK(oldGlyphs[i].destRect.h == newGlyphs[i].destRect.h);

				//CHECK(oldGlyphs[i].destRect.x != newGlyphs[i].destRect.x);
			}

			renderOriginalTransparent();
			RenderTextGlyphs(std::format("GlyphCacheHandler_change_text_align_{}.png", alignStr),
				glyphCache, textRenderable, transform, glyphAtlas.GetSourceTexture());
		}

		SDLite::Exit();
	}
}