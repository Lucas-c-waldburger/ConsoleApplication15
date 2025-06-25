#pragma once
#include <SDL.h>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include "../ecs/Ecs.h"
#include "../atlas/AtlasManager.h"
#include "../components/RenderableComponent.h"
#include "../core/ScopedInvoker.h"
#include "../core/commonObjects.h"

class RenderContext
{
public:

private:
	enum Option : uint8_t
	{
		ApplyOpacity = 1 << 0,
		ApplyBlendMode = 1 << 1,
		ApplyColorMod = 1 << 2
	};

	SDL_Texture* texture = nullptr;
    SDL_Rect srcRect;
    SDL_Rect destRect;
    Renderable* renderable = nullptr;
	uint8_t options = 0;
};

template <typename RenderFn> requires std::is_invocable_v<RenderFn, const Renderable&>
void Render(const Renderable& renderable, RenderFn&& fn)
{

}

struct RenderCache
{
    SDL_Texture* lastTexture = nullptr;
    TextureModsOld lastMods;

    void Update(SDL_Texture* texture, const TextureModsOld& mods) 
    {
        if (texture != lastTexture || lastMods.blend != mods.blend) 
        {
            SDL_SetTextureBlendMode(texture, mods.blend);
        }
        if (texture != lastTexture || lastMods.alpha != mods.alpha) 
        {
            SDL_SetTextureAlphaMod(texture, mods.alpha);
        }
        if (texture != lastTexture || lastMods.color != mods.color) 
        {
            SDL_SetTextureColorMod(texture, mods.color.r, mods.color.g, mods.color.b);
        }

        lastTexture = texture;
        lastMods = mods;
    }
};


class RenderPipeline
{
public:
    

private:
    void Reset()
    {
    }

    impl::TextureManager* atlasStore_;
    RenderCache cache_;
    TextureModsOld initialMods_;
};

//class RenderOp
//{
//public:
//	explicit RenderOp(SpriteRenderContext& ctx) : parent_(&ctx) {}
//
//private:
//	SpriteRenderContext* parent_ = nullptr;
//};

//struct RenderOp
//{
//    std::function<void()> fn;
//
//    void operator()() const { if (fn) fn(); }
//
//    RenderOp operator|(const RenderOp& other) const 
//    {
//        return RenderOp{ [a = *this, b = other]() { a(); b(); } };
//    }
//
//	static RenderOp WithOpacity(SDL_Texture* texture, uint8_t alpha)
//	{
//        return RenderOp{ [=]() 
//        {
//            Uint8 oldAlpha;
//            SDL_GetTextureAlphaMod(texture, &oldAlpha);
//            SDL_SetTextureAlphaMod(texture, alpha);
//
//            ScopedInvoker guard{ [texture, oldAlpha]() { 
//                SDL_SetTextureAlphaMod(texture, oldAlpha); 
//            }};
//        }
//        };
//	}
//
//    static RenderOp WithTint(SDL_Texture* texture, uint8_t r, uint8_t g, uint8_t b)
//    {
//        return RenderOp{ [=]()
//        {
//            Uint8 oldR, oldG, oldB;
//            SDL_GetTextureColorMod(texture, &oldR, &oldG, &oldB);
//
//            SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
//            SDL_SetTextureColorMod(texture, r, g, b);
//
//            ScopedInvoker guard{ [texture, oldR, oldG, oldB]() {
//                SDL_SetTextureColorMod(texture, oldR, oldG, oldB);
//            }};
//        }
//        };
//    }
//
//    static RenderOp WithBlendMode(SDL_Texture* texture, SDL_BlendMode blendMode)
//    {
//        return RenderOp{ [=]()
//        {
//            SDL_BlendMode oldBlendMode;
//            SDL_GetTextureBlendMode(texture, &oldBlendMode);
//
//            SDL_SetTextureBlendMode(texture, blendMode);
//
//            ScopedInvoker guard{ [texture, oldBlendMode]() {
//                SDL_SetTextureBlendMode(texture, oldBlendMode);
//            }};
//        }
//        };
//    }
//};