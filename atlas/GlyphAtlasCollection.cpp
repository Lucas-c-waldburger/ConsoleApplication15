#include "GlyphAtlasCollection.h"
#include "PackingTools.h"
#include "../core/ScopedInvoker.h"
#include <SDL_ttf.h>
#include <filesystem>
#include <ranges>


namespace {

static constexpr size_t kNumGlyphs = static_cast<size_t>(
    FontAtlasTexture::kEndChar - FontAtlasTexture::kStartChar
);
static constexpr SDL_Color kWhite = { 255, 255, 255, 255 };

static constexpr size_t GetPlotIndexForChar(char c)
{
    if (c < FontAtlasTexture::kStartChar || c > FontAtlasTexture::kEndChar)
    {
        return std::numeric_limits<size_t>::max();
    }

    return static_cast<size_t>(c - FontAtlasTexture::kStartChar);
}

Result<Void> PrepareFontDescriptor(FontDescriptor& descriptor)
{
    if (descriptor.fontSize <= 0)
    {
        return MAKE_ERROR_FMT("Invalid font size: '{}'", descriptor.fontSize);
    }
    if (descriptor.filepath.empty())
    {
        return MAKE_ERROR("Font filepath was empty");
    }
    if (descriptor.fontName.empty())
    {
        descriptor.fontName =
            std::filesystem::path(descriptor.filepath).stem().string();
    }

    return Void{};
}

} // unnamed namespace 

Result<FontAtlasTexture> FontAtlasTexture::Create(SDL_Renderer* renderer,
                                            FontDescriptor& descriptor)
{
    FontAtlasTexture glyphAtlas{ GetNextAtlasID() };

    TRY(glyphAtlas.LoadImpl(renderer, descriptor));

    return glyphAtlas;
}

Result<Void> FontAtlasTexture::LoadImpl(SDL_Renderer* renderer,
                                     FontDescriptor& descriptor)
{
    TTF_Font* font = TTF_OpenFont(descriptor.filepath.c_str(),
                                  descriptor.fontSize);
    if (!font)
    {
        return MAKE_ERROR_FMT("Failed to load font: {}", TTF_GetError());
    }

    fontHeight_ = TTF_FontHeight(font);
    assert(fontHeight_ > 0);
    descriptor.fontHeight = fontHeight_;

    std::vector<GlyphSurface> glyphSurfaces{ kNumGlyphs };
    SDL_Surface* atlasSurface = nullptr;

    ScopedInvoker freeResources{ [&] {
        if (font) { TTF_CloseFont(font); }
        for (auto& [_, surf] : glyphSurfaces) { SDL_FreeSurface(surf); }
        SDL_FreeSurface(atlasSurface);
    } };

    // fill glyph metrics and calculate total area for atlas
    int totalArea = 0;
    for (char c = kStartChar; c < kEndChar; c++)
    {
        auto& [glyph, surface] = glyphSurfaces[GetPlotIndexForChar(c)];

        glyph.character = c;

        if (TTF_GlyphMetrics32(font, c, nullptr, nullptr, nullptr,
            nullptr, &glyph.advance) != 0)
        {
            return MAKE_ERROR_FMT("Failed to get glyph metrics for char '{}': {}",
                c, TTF_GetError());
        }

        surface = TTF_RenderGlyph_Blended(font, c, kWhite);
        if (!surface)
        {
            return MAKE_ERROR_FMT("Failed to make surface for char '{}': {}",
                c, TTF_GetError());
        }

        if (surface->w <= 0 || surface->h <= 0)
        {
            return MAKE_ERROR_FMT("Surface dimensions invalid: ({}, {})",
                surface->w, surface->h);
        }

        totalArea += surface->w * surface->h;
    }

    float idealSideLen = std::ceil(std::sqrt(static_cast<float>(totalArea)));
    int atlasSideLen = GetNextPowerOfTwo(static_cast<int>(idealSideLen));

    // assign each glyph to a plot on the atlas texture
    bool done = false;
    while (!done)
    {
        done = true;

        if (atlasSideLen > (1 << 30))
        {
            return MAKE_ERROR("Not all rects could be packed "
                "in the maximum atlas size");
        }

        binPack_.Init(atlasSideLen, atlasSideLen, false);

        for (auto& [glyph, surface] : glyphSurfaces)
        {
            rbp::Rect packed = binPack_.Insert(
                surface->w, surface->h, rbp::MaxRectsBinPack::RectBestAreaFit
            );
            if (!WasRectPacked(packed))
            {
                atlasSideLen *= 2;

                done = false;

                break;
            }

            glyph.plot.rect = RbpToSDLRect(packed);
        }
    }

    // make atlas texture, blit the glyph surfaces on it
    atlasSurface = SDL_CreateRGBSurfaceWithFormat(
        0, atlasSideLen, atlasSideLen, 32, SDL_PIXELFORMAT_RGBA32
    );

    textureSize_ = static_cast<size_t>(atlasSideLen);

    SDL_FillRect(atlasSurface, nullptr,
        SDL_MapRGBA(atlasSurface->format, 0, 0, 0, 0));

    for (auto&& [glyph, surface] : glyphSurfaces)
    {
        if (SDL_BlitSurface(surface, nullptr, atlasSurface, &glyph.plot.rect) < 0)
        {
            return MAKE_ERROR_FMT("Failed to blit surface: {}", SDL_GetError());
        }

        SDL_FreeSurface(surface);

        glyphs_.emplace_back(glyph);
    }

    atlasTexture_ = MakeUniqueTexturePtrFromSurface(renderer, atlasSurface);
    if (!atlasTexture_)
    {
        return MAKE_ERROR_FMT("Failed to create atlas texture: {}", SDL_GetError());
    }

    SDL_SetTextureBlendMode(atlasTexture_.get(), SDL_BLENDMODE_BLEND);

    SDL_FreeSurface(atlasSurface);
    TTF_CloseFont(font);

    freeResources.Release();

    return Void{};
}

Glyph FontAtlasTexture::GetGlyph(char c) const
{
    if (c == '\n') { return kNewlineGlyph; }

    const size_t idx = GetPlotIndexForChar(c);

    return (idx < glyphs_.size()) ? glyphs_[idx] : Glyph{};
}

std::vector<Glyph> FontAtlasTexture::GetGlyphsForString(std::string_view text) const
{
    std::vector<Glyph> glyphs;
    glyphs.reserve(text.size());

    for (char c : text)
    {
        glyphs.emplace_back(GetGlyph(c));
    }

    return glyphs;
}

Result<Void> FontAtlasTexture::RebuildSourceTexture(SDL_Renderer* renderer, 
                                                 std::string_view filepath,
                                                 int fontSize)
{
    atlasTexture_ = nullptr;

    TTF_Font* font = TTF_OpenFont(filepath.data(), fontSize);
    if (!font)
    {
        return MAKE_ERROR_FMT("Failed to load font: {}", TTF_GetError());
    }

    SDL_Surface* atlasSurface = SDL_CreateRGBSurfaceWithFormat(
        0, static_cast<int>(textureSize_), static_cast<int>(textureSize_), 
        32, SDL_PIXELFORMAT_RGBA32
    );

    SDL_FillRect(atlasSurface, nullptr,
        SDL_MapRGBA(atlasSurface->format, 0, 0, 0, 0));

    SDL_Surface* glyphSurface = nullptr;

    ScopedInvoker freeResources{ [&] {
        if (font) { TTF_CloseFont(font); }
        SDL_FreeSurface(glyphSurface);
        SDL_FreeSurface(atlasSurface);
    } };

    for (auto& glyph : glyphs_)
    {
        glyphSurface = TTF_RenderGlyph_Blended(font, glyph.character, kWhite);
        if (!glyphSurface)
        {
            return MAKE_ERROR_FMT("Failed to make surface for char '{}': {}",
                glyph.character, TTF_GetError());
        }

        if (glyphSurface->w != glyph.plot.rect.w || 
            glyphSurface->h != glyph.plot.rect.h)
        {
            return MAKE_ERROR_FMT("Surface dimensions '({}, {})' did not match "
                "plot dimensions: '({}, {})'", glyphSurface->w, glyphSurface->h,
                glyph.plot.rect.w, glyph.plot.rect.h);
        }

        if (SDL_BlitSurface(glyphSurface, nullptr, atlasSurface, &glyph.plot.rect) < 0)
        {
            return MAKE_ERROR_FMT("Failed to blit surface: {}", SDL_GetError());
        }

        SDL_FreeSurface(glyphSurface);
    }

    atlasTexture_ = MakeUniqueTexturePtrFromSurface(renderer, atlasSurface);
    if (!atlasTexture_)
    {
        return MAKE_ERROR_FMT("Failed to create atlas texture: {}", SDL_GetError());
    }

    SDL_SetTextureBlendMode(atlasTexture_.get(), SDL_BLENDMODE_BLEND);

    SDL_FreeSurface(atlasSurface);
    TTF_CloseFont(font);

    freeResources.Release();

    return Void{};
}

// FONT ATLAS
//FontAtlas::FontAtlas(FontAtlas&& other) noexcept :
//    TextureCreationNotifier(std::move(other)),
//    fontAtlasTextures_(std::move(other.fontAtlasTextures_)),
//    fontInfo_(std::move(other.fontInfo_))
//{
//    RepopulateFontNameIndexMap(other.fontNameIndices_.size());
//}
//
//FontAtlas& FontAtlas::operator=(FontAtlas&& other) noexcept
//{
//    if (this == &other)
//    {
//        return *this;
//    }
//
//    TextureCreationNotifier::operator=(std::move(other));
//    fontAtlasTextures_ = std::move(other.fontAtlasTextures_);
//    fontInfo_ = std::move(other.fontInfo_);
//
//    RepopulateFontNameIndexMap(other.fontNameIndices_.size());
//
//    return *this;
//}

Result<Handle<TextureResource>> FontAtlas::LoadFont(SDL_Renderer* renderer, 
								                    FontDescriptor&& fontDescriptor)
{
    TRY(PrepareFontDescriptor(fontDescriptor));

	if (fontNameIndices_.contains(fontDescriptor.fontName))
	{
		return MAKE_ERROR_FMT("Duplicate font name: '{}'", fontDescriptor.fontName);
	}

	TRY_ASSIGN(fontAtlasTextures_.emplace_back(),
		FontAtlasTexture::Create(renderer, fontDescriptor));

	auto& newAtlas = fontAtlasTextures_.back();

	NotifyTextureCreated(newAtlas.GetAtlasID(), newAtlas.GetSourceTexture());

    //bool needRepopulateViews = fontInfo_.Size() == fontInfo_.Capacity();
    //if (needRepopulateViews)
    //{
    //    const size_t newSize = fontInfo_.Size() + kDefaultFrontInfoCapacity;

    //    fontInfo_.Reserve(newSize);
    //    RepopulateFontNameIndexMap(newSize);
    //}

    const size_t fontIdx = fontInfo_.PushBack(FontInfo{
        .atlasId = newAtlas.GetAtlasID(),
        .fontName = std::move(fontDescriptor.fontName),
        .filepath = std::move(fontDescriptor.filepath),
        .fontSize = fontDescriptor.fontSize,
        .fontHeight = fontDescriptor.fontHeight
    });

	auto [_, inserted] = fontNameIndices_.try_emplace(
        fontInfo_.GetView<&FontInfo::fontName>(fontIdx), fontIdx
    );
    assert(inserted);

    return Handle<TextureResource>::Create(newAtlas.GetAtlasID(), fontIdx);
}

Result<Void> FontAtlas::LoadFonts(SDL_Renderer* renderer, 
                                  FontDescriptors&& fontDescriptors)
{
    if (fontDescriptors.empty())
    {
        return MAKE_ERROR("FontDescriptors were empty");
    }

    for (auto&& descriptor : fontDescriptors)
    {
        TRY(LoadFont(renderer, std::move(descriptor)));
    }

    return kVoid;
}

const FontAtlasTexture& FontAtlas::GetFont(std::string_view fontName) const
{
	auto it = fontNameIndices_.find(fontName);
	if (it == fontNameIndices_.end())
	{
        return kInvalidGlyphAtlas;
	}

	assert(it->second < fontAtlasTextures_.size());
    assert(fontInfo_.Size() == fontAtlasTextures_.size());
    assert(fontName == fontInfo_.GetView<&FontInfo::fontName>(it->second));

	return fontAtlasTextures_[it->second];
}

const FontAtlasTexture& 
FontAtlas::GetFont(const Handle<TextureResource>& handle) const
{
    const size_t fontIdx = static_cast<size_t>(handle.GetResourceIndex());
    if (fontIdx >= fontAtlasTextures_.size())
    {
        return kInvalidGlyphAtlas;
    }

    assert(fontInfo_.Size() == fontAtlasTextures_.size());
    assert(handle.GetAtlasID() == fontAtlasTextures_[fontIdx].GetAtlasID());

    return fontAtlasTextures_[fontIdx];
}

bool FontAtlas::HasFont(std::string_view fontName) const
{
	return fontNameIndices_.contains(fontName);
}

bool FontAtlas::HasFont(const Handle<TextureResource>& handle) const
{
    assert(fontInfo_.Size() == fontAtlasTextures_.size());

    const size_t fontIdx = static_cast<size_t>(handle.GetResourceIndex());

    if (handle.GetResourceIndex() >= fontInfo_.Size())
    {
        return false;
    }

    return fontIdx < fontInfo_.Size() && 
           fontAtlasTextures_[fontIdx].GetAtlasID() == handle.GetAtlasID();
}

GlyphTextWriter FontAtlas::GetTextWriter(std::string_view fontName) const
{
    auto it = fontNameIndices_.find(fontName);
    if (it == fontNameIndices_.end())
    {
        return {};
    }

    assert(it->second < fontAtlasTextures_.size());
    assert(fontInfo_.Size() == fontAtlasTextures_.size());
    assert(fontName == fontInfo_.GetView<&FontInfo::fontName>(it->second));

    const auto handle = Handle<TextureResource>::Create(
        fontAtlasTextures_[it->second].GetAtlasID(), it->second
    );

    return GlyphTextWriter{ .resourceHandle = handle };
}

bool FontAtlas::IsTextWriterValid(const GlyphTextWriter& writer) const
{
    return HasFont(writer.resourceHandle);
}

Result<Void> FontAtlas::RebuildSourceTextures(SDL_Renderer* renderer)
{
    assert(fontInfo_.Size() == fontAtlasTextures_.size());

    for (size_t i = 0; i < fontInfo_.Size(); ++i)
    {
        const auto& [filepath, fontSize] = fontInfo_.GetView<&FontInfo::filepath,
                                                             &FontInfo::fontSize>(i);
            
        TRY(fontAtlasTextures_[i].RebuildSourceTexture(renderer, filepath, fontSize));

        NotifyTextureCreated(fontAtlasTextures_[i].GetAtlasID(),
                             fontAtlasTextures_[i].GetSourceTexture());
    }

    return kVoid;
}

std::vector<FontDescriptor> FontAtlas::ExportFontDescriptors() const
{
    std::vector<FontDescriptor> descriptors;
    descriptors.reserve(fontInfo_.Size());

    auto iter = fontInfo_.ForEach<&FontInfo::fontName, &FontInfo::filepath, 
                                  &FontInfo::fontSize, &FontInfo::fontHeight>();

    for (const auto [name, path, size, height] : iter)
    {
        descriptors.emplace_back(name, path, size, height);
    }

    return descriptors;
}

size_t FontAtlas::GetTextureCount() const
{
    return fontAtlasTextures_.size();
}

//void FontAtlas::RepopulateFontNameIndexMap(size_t newSize)
//{
//    fontNameIndices_.clear();
//    fontNameIndices_.reserve(newSize);
//
//    for (size_t i = 0; i < fontInfo_.Size(); ++i)
//    {
//        const auto& name = fontInfo_.GetView<&FontInfo::fontName>(i);
//
//        auto [_, inserted] = fontNameIndices_.try_emplace(name, i);
//        assert(inserted);
//    }
//}