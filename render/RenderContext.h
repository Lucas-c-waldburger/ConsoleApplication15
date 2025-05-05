#pragma once
#include <SDL.h>
#include <functional>
#include "../components/RenderableComponent.h"
#include "../core/ScopedInvoker.h"

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

//class RenderOp
//{
//public:
//	explicit RenderOp(RenderContext& ctx) : parent_(&ctx) {}
//
//private:
//	RenderContext* parent_ = nullptr;
//};

struct RenderOp
{
    std::function<void()> fn;

    void operator()() const { if (fn) fn(); }

    RenderOp operator|(const RenderOp& other) const 
    {
        return RenderOp{ [a = *this, b = other]() { a(); b(); } };
    }

	static RenderOp WithOpacity(SDL_Texture* texture, uint8_t alpha)
	{
        return RenderOp{ [=]() 
        {
            Uint8 oldAlpha;
            SDL_GetTextureAlphaMod(texture, &oldAlpha);
            SDL_SetTextureAlphaMod(texture, alpha);

            ScopedInvoker guard{ [texture, oldAlpha]() { 
                SDL_SetTextureAlphaMod(texture, oldAlpha); 
            }};
        }
        };
	}

    static RenderOp WithTint(SDL_Texture* texture, uint8_t r, uint8_t g, uint8_t b)
    {
        return RenderOp{ [=]()
        {
            Uint8 oldR, oldG, oldB;
            SDL_GetTextureColorMod(texture, &oldR, &oldG, &oldB);

            SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
            SDL_SetTextureColorMod(texture, r, g, b);

            ScopedInvoker guard{ [texture, oldR, oldG, oldB]() {
                SDL_SetTextureColorMod(texture, oldR, oldG, oldB);
            }};
        }
        };
    }

    static RenderOp WithBlendMode(SDL_Texture* texture, SDL_BlendMode blendMode)
    {
        return RenderOp{ [=]()
        {
            SDL_BlendMode oldBlendMode;
            SDL_GetTextureBlendMode(texture, &oldBlendMode);

            SDL_SetTextureBlendMode(texture, blendMode);

            ScopedInvoker guard{ [texture, oldBlendMode]() {
                SDL_SetTextureBlendMode(texture, oldBlendMode);
            }};
        }
        };
    }
};