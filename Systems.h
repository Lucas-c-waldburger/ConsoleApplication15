#pragma once
#include "Ecs.h"
#include "SDLite.h"
#include "SDLUtils.h"
#include "GlyphAtlas.h"
#include "core/Result.h"
#include "core/Aliases.h"
#include "core/maths.h"
#include <span>

template <typename T> struct Sentinel;

template <typename T> static constexpr T sentinel_v = Sentinel<T>::value;

template <typename T> requires (std::is_arithmetic_v<T> && std::is_signed_v<T>)
struct Sentinel<T> { static constexpr T value = std::numeric_limits<T>::min(); };

template <typename T>
struct Sentinel<Dimensions<T>>
{
	static constexpr Dimensions<T> value = { sentinel_v<T>, sentinel_v<T> };
};



class RenderSystem
{
public:
	using Alignment = Renderable::Text::Alignment;

	struct RenderGlyphsArgs
	{
		SDL_Renderer* renderer = nullptr;
		GlyphAtlas& glyphAtlas;
		std::string_view text;
		SDL_FPoint scale = { 0.0f, 0.0f };
		int numNewlines = 0;
		int startY = 0;
		int startX = 0;
	};

	void Update(SDL_Renderer* renderer)
	{
		auto entities = ECS::GetAllEntitiesWith<Spatial, Transform, Renderable>();

		std::sort(entities.begin(), entities.end(), [](const Entity& lhs, const Entity& rhs) {
			return lhs.GetComponent<Renderable>().drawOrder < 
				   rhs.GetComponent<Renderable>().drawOrder;				
		});

		for (auto& entity : entities)
		{
			auto [renderable, spatial, transform] = entity.GetComponents<Renderable, Spatial, Transform>();

			SDL_Rect renderRect = MakeTransformedRect(spatial, transform);

			if (auto spriteData = std::get_if<Renderable::Sprite>(&renderable.renderData))
			{
				auto atlasIt = spriteSeriesAtlases_.find(spriteData->sourceAtlas);
				if (atlasIt == spriteSeriesAtlases_.end())
				{
					std::cerr << "Sprite handle expired for sprite { seriesName : " << spriteData->seriesName
							  << ", index : " << spriteData->currentIndex << " }\n";

					continue;
				}

				auto& spriteAtlas = atlasIt->second;

				SDL_Rect srcRect = spriteAtlas.GetSprite(spriteData->seriesName, 
														 spriteData->currentIndex).atlasRect;
				if (srcRect.w == 0 || srcRect.h == 0)
				{
					std::cerr << "SpriteInfo not found for sprite { seriesName : " << spriteData->seriesName
							  << ", index : " << spriteData->currentIndex << " }\n";

					continue;
				}

				SDL_RenderCopyEx(renderer, spriteAtlas.GetAtlasTexture(), &srcRect, 
							     &renderRect, transform.rotation, nullptr, renderable.flip);
			}

			else if (auto textData = std::get_if<Renderable::Text>(&renderable.renderData))
			{
				if (textData->text.empty()) { continue; }

				auto atlasIt = glyphAtlases_.find(textData->sourceAtlas);
				if (atlasIt == glyphAtlases_.end())
				{
					std::cerr << "Glyph handle expired for text { \"" << textData->text << " }\n";

					continue;
				}

				auto& glyphAtlas = atlasIt->second;

				const int numNewlines = std::count(textData->text.begin(), textData->text.end(), '\n');
				const int totalHeight = (glyphAtlas.GetAtlasInfo().fontHeight * transform.scale.y) * 
										(numNewlines + 1);

				RenderGlyphsArgs args{
					.renderer = renderer,
					.glyphAtlas = glyphAtlas,
					.text = textData->text,
					.scale = transform.scale,
					.numNewlines = numNewlines,
					.startY = static_cast<int>(spatial.position.y - (totalHeight / 2.0f))
				};

				if (textData->scaleToFit)
				{
					float toFit = GetScaleToFitFactor(args, totalHeight, renderRect.w, renderRect.h);

					args.scale.x *= toFit;
					args.scale.y *= toFit;
				}

				switch (textData->align)
				{
				case Alignment::Left:
					args.startX = renderRect.x;
					RenderGlyphsAligned<Alignment::Left>(args);
					break;

				case Alignment::Right:
					args.startX = renderRect.x + renderRect.w;
					RenderGlyphsAligned<Alignment::Right>(args);
					break;

				case Alignment::Center:
					args.startX = spatial.position.x;
					RenderGlyphsAligned<Alignment::Center>(args);
					break;
				}
			}

			else
			{
				std::cerr << "Logic error: renderable type not text or sprite";
			}
		}
	}
	
	template <typename T> requires (std::same_as<T, SpriteSeriesAtlas> || std::same_as<T, GlyphAtlas>)
	Result<Handle<T>> LoadAtlas(SDL_Renderer* renderer, HandleManager& handleManager, AtlasInfo<T> atlasInfo)
	{
		if constexpr (std::same_as<T, SpriteSeriesAtlas>)
		{
			return LoadAtlasImpl(renderer, handleManager, std::move(atlasInfo), spriteSeriesAtlases_);
		}
		else
		{
			return LoadAtlasImpl(renderer, handleManager, std::move(atlasInfo), glyphAtlases_);
		} 
	}

	template <typename T> requires (std::same_as<T, SpriteSeriesAtlas> || std::same_as<T, GlyphAtlas>)
	T* GetAtlas(const Handle<T>& handle)
	{
		if constexpr (std::same_as<T, SpriteSeriesAtlas>)
		{
			return GetAtlasImpl(handle, spriteSeriesAtlases_);
		}
		else
		{
			return GetAtlasImpl(handle, glyphAtlases_);
		}
	}

private:
	template <typename T>
	static Result<Handle<T>> LoadAtlasImpl(SDL_Renderer* renderer, HandleManager& handleManager, 
										   AtlasInfo<T>&& atlasInfo, std::unordered_map<Handle<T>, T>& atlasMap)
	{
		
		T atlas{ handleManager.GetHandle<T>() };

		auto handle = atlas.GetHandle();

		auto [it, inserted] = atlasMap.emplace(handle, std::move(atlas));
		if (!inserted)
		{
			return MAKE_ERROR("Repeat handle generated for atlas");
		}

		bool loadResult = it->second.Load(renderer, std::move(atlasInfo));
		if (!loadResult)
		{
			return MAKE_ERROR("Atlas load failed");
		}

		return handle;
	}

	template <typename T>
	static T* GetAtlasImpl(const Handle<T>& handle, std::unordered_map<Handle<T>, T>& atlasMap)
	{
		auto it = atlasMap.find(handle);
		return (it != atlasMap.end()) ? &it->second : nullptr;
	}

	static float GetScaleToFitFactor(const RenderGlyphsArgs& args, int totalHeight,
									 int boundingWidth, int boundingHeight)
	{
		auto glyphs = args.glyphAtlas.GetGlyphsForString(args.text);
		assert(glyphs.size() == args.text.size());

		int currentPos = 0;
		int longestRowWidth = 0;
		for (int i = 0; i <= args.numNewlines; i++)
		{
			size_t newlinePos = args.text.find_first_of('\n', currentPos);
			newlinePos = (std::min(newlinePos, args.text.length()));

			int rowWidth = std::accumulate(
				glyphs.begin() + currentPos,
				glyphs.begin() + newlinePos,
				0, [scale = args.scale.x](int sum, const auto& glyph) {
					return sum + (glyph.advance * scale);
				});

			longestRowWidth = std::max(longestRowWidth, rowWidth);

			currentPos = newlinePos + 1;
		}
		
		return std::min(boundingWidth / static_cast<float>(longestRowWidth),
						boundingHeight / static_cast<float>(totalHeight));
	}

	template <Alignment>
	static void RenderGlyphsAligned(const RenderGlyphsArgs& args);

	template <>
	static void RenderGlyphsAligned<Alignment::Left>(const RenderGlyphsArgs& args)
	{
		int xPos = args.startX;
		int yPos = args.startY;

		for (int i = 0; i < args.text.size(); i++)
		{
			if (args.text[i] == '\n')
			{
				xPos = args.startX;
				yPos += args.glyphAtlas.GetAtlasInfo().fontHeight;

				continue;
			}

			auto glyph = args.glyphAtlas[args.text[i]];
			assert(glyph.character != kInvalidChar);

			SDL_Rect dest = { xPos, yPos, glyph.atlasRect.w * args.scale.x,
										  glyph.atlasRect.h * args.scale.y };

			SDL_RenderCopy(args.renderer, args.glyphAtlas.GetAtlasTexture(), 
						   &glyph.atlasRect, &dest);

			xPos += glyph.advance * args.scale.x;
		}
	}

	template <>
	static void RenderGlyphsAligned<Alignment::Right>(const RenderGlyphsArgs& args)
	{
		int xPos = args.startX;
		int yPos = args.startY;

		for (int i = args.text.size() - 1; i >= 0; i--)
		{
			if (args.text[i] == '\n')
			{
				xPos = args.startX;
				yPos += args.glyphAtlas.GetAtlasInfo().fontHeight;

				continue;
			}

			auto glyph = args.glyphAtlas[args.text[i]];
			assert(glyph.character != kInvalidChar);

			SDL_Rect dest = { xPos - (glyph.atlasRect.w * args.scale.x), yPos,
							  glyph.atlasRect.w * args.scale.x,
							  glyph.atlasRect.h * args.scale.y };

			SDL_RenderCopy(args.renderer, args.glyphAtlas.GetAtlasTexture(),
						   &glyph.atlasRect, &dest);

			xPos -= glyph.advance * args.scale.x;
		}
	}

	template <>
	static void RenderGlyphsAligned<Alignment::Center>(const RenderGlyphsArgs& args)
	{
		auto glyphs = args.glyphAtlas.GetGlyphsForString(args.text);
		assert(glyphs.size() == args.text.size());

		int yPos = args.startY;
		int currentPos = 0;
		for (int i = 0; i <= args.numNewlines; i++)
		{
			size_t newlinePos = args.text.find_first_of('\n', currentPos);
			newlinePos = (std::min(newlinePos, args.text.length()));

			int rowWidth = std::accumulate(
				glyphs.begin() + currentPos,
				glyphs.begin() + newlinePos,
				0, [scale = args.scale.x](int sum, const auto& glyph) {
					return sum + (glyph.advance * scale);
				});

			int xPos = args.startX - (rowWidth / 2.0f);

			for (int j = currentPos; j < newlinePos; j++) 
			{
				auto& glyph = glyphs[j];
				assert(glyph.character != kInvalidChar);

				SDL_Rect dest = { xPos , yPos, glyph.atlasRect.w * args.scale.x,
											   glyph.atlasRect.h * args.scale.y };

				SDL_RenderCopy(args.renderer, args.glyphAtlas.GetAtlasTexture(),
							   &glyph.atlasRect, &dest);

				xPos += glyph.advance * args.scale.x;
			}

			yPos += args.glyphAtlas.GetAtlasInfo().fontHeight * args.scale.y;
			currentPos = newlinePos + 1;
		}
	}

	std::unordered_map<Handle<GlyphAtlas>, GlyphAtlas> glyphAtlases_;
	std::unordered_map<Handle<SpriteSeriesAtlas>, SpriteSeriesAtlas> spriteSeriesAtlases_;
};


//for (int i = 0; i < textData->text.size(); i++)
//{
//	if (textData->text[i] == '\n') 
//	{
//		xPos = renderRect.x;
//		yPos += glyphAtlas.GetAtlasInfo().fontHeight;

//		continue;
//	}

//	auto glyph = glyphAtlas[textData->text[i]];
//	assert(glyph.character != kInvalidChar);

//	SDL_Rect dest = { xPos, yPos, glyph.atlasRect.w * transform.scale,
//								  glyph.atlasRect.h * transform.scale };

//	SDL_RenderCopy(SDLite::Renderer(), atlasIt->second.GetAtlasTexture(),
//		&glyph.atlasRect, &dest);

//	xPos += glyph.advance * transform.scale;
//}

//// right align
//int xPos = renderRect.x + renderRect.w;
//int yPos = spatial.position.y - (totalHeight / 2.0f);

//for (int i = textData->text.size() - 1; i >= 0; i--)
//{
//	if (textData->text[i] == '\n')
//	{
//		xPos = renderRect.x + renderRect.y;
//		yPos += glyphAtlas.GetAtlasInfo().fontHeight;

//		continue;
//	}

//	auto glyph = glyphAtlas[textData->text[i]];
//	assert(glyph.character != kInvalidChar);

//	xPos -= glyph.advance * transform.scale;

//	SDL_Rect dest = { xPos - (glyph.atlasRect.w * transform.scale), yPos,
//					  glyph.atlasRect.w * transform.scale,
//					  glyph.atlasRect.h * transform.scale };

//	SDL_RenderCopy(SDLite::Renderer(), atlasIt->second.GetAtlasTexture(),
//		&glyph.atlasRect, &dest);
//}

//// center align
//auto glyphs = glyphAtlas.GetGlyphsForString(textData->text);
//assert(glyphs.size() == textData->text.size());

//int xPos = 0;
//int currentPos = 0;
//for (int i = 0; i <= numNewlines; i++)
//{
//	size_t newlinePos = textData->text.find_first_of('\n', currentPos);
//	newlinePos = (std::min(newlinePos, textData->text.length() - 1));

//	int rowWidth = std::accumulate(
//		glyphs.begin() + currentPos,
//		glyphs.begin() + newlinePos,
//		0, [scale = transform.scale](int sum, const auto& glyph) {
//			return sum + (glyph.advance * scale);
//		});

//	int xPos = spatial.position.x - (rowWidth / 2.0f);
//			 
//	for (int j = currentPos; j < newlinePos; j++)
//	{
//		auto& glyph = glyphs[j];
//		assert(glyph.character != kInvalidChar);

//		SDL_Rect dest = { xPos , yPos, glyph.atlasRect.w * transform.scale,
//									   glyph.atlasRect.h * transform.scale };

//		SDL_RenderCopy(SDLite::Renderer(), glyphAtlas.GetAtlasTexture(),
//						&glyph.atlasRect, &dest);

//		xPos += glyph.advance * transform.scale;
//	}
//	
//	yPos += glyphAtlas.GetAtlasInfo().fontHeight * transform.scale;
//	currentPos = newlinePos + 1;
//}
//                        
//int xPos = renderRect.x + renderRect.w;
//int yPos = spatial.position.y - (totalHeight / 2.0f);

//for (int i = textData->text.size() - 1; i >= 0; i--)
//{
//	if (textData->text[i] == '\n')
//	{
//		xPos = xPos = renderRect.x + renderRect.y;
//		yPos += glyphAtlas.GetAtlasInfo().fontHeight;

//		continue;
//	}

//	auto glyph = glyphAtlas[textData->text[i]];
//	assert(glyph.character != kInvalidChar);

//	xPos -= glyph.advance * transform.scale;

//	SDL_Rect dest = { xPos - (glyph.atlasRect.w * transform.scale), yPos,
//					  glyph.atlasRect.w * transform.scale,
//					  glyph.atlasRect.h * transform.scale };

//	SDL_RenderCopy(SDLite::Renderer(), atlasIt->second.GetAtlasTexture(),
//		&glyph.atlasRect, &dest);
//}