#include "../CatchMain.cpp"
#include "../../Fixtures.h"

static constexpr std::string_view kTestImagesPath = "test_images/";

Result<Void> WriteFrameBufferToPNG(SDL_Renderer* renderer, SDL_Texture* texture, 
								   std::string_view fileName)
{
	if (!texture)
	{
		return MAKE_ERROR("SDL Texture was null");
	}

	int w, h;
	SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);

	SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_RGBA32);
	if (!surface)
	{
		return MAKE_ERROR(SDL_GetError());
	}

	SDL_SetRenderTarget(renderer, texture);

	SDL_RenderReadPixels(renderer, nullptr, SDL_PIXELFORMAT_RGBA32, 
						 surface->pixels, surface->pitch);

	if (IMG_SavePNG(surface, std::format("{}{}", kTestImagesPath, fileName).c_str()) != 0)
	{
		return MAKE_ERROR(IMG_GetError());
	}

	SDL_FreeSurface(surface);
}

TEST_CASE("Sprites rendered correctly", "[renderer]")
{
	auto fixtureResult = SceneFixture::GetInstance();
	REQUIRE(fixtureResult.Success());

	auto& fixture = fixtureResult.GetValue();

	SpriteSeriesResourcePacket sprites{};
	//sprites.instances.push_back(SpriteInstanceData{.})
}