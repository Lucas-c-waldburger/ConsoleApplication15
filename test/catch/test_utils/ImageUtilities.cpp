#include "ImageUtilities.h"
#include <SDL_image.h>
#include <cassert>
#include "../../../file/FilePathUtility.h"

namespace test {

namespace {

class PlaceholderImageFilename
{
public:
	static std::string Get() { return std::format(kNameFmt, counter_++); }

private:
	PlaceholderImageFilename() = default;

	static constexpr std::string_view kNameFmt = "placeholder_image_name_{}.png";
	static inline size_t counter_ = 0;
};

std::string MakeTestImageFilepathString(std::string_view imgFilename)
{
	namespace fs = std::filesystem;

	auto testImagesDirPath = FilePathUtility::GetRootPath() / kTestImagesDirPath;

	assert(fs::exists(testImagesDirPath));
	assert(fs::is_directory(testImagesDirPath));

	std::string imagePathStr = (!imgFilename.empty())
		? (testImagesDirPath / fs::path(imgFilename)).string()
		: (testImagesDirPath / fs::path(PlaceholderImageFilename::Get())).string();

	const size_t dotPos = imagePathStr.find_last_of('.');
	if (dotPos == std::string::npos)
	{
		imagePathStr += kPngExtension;
	}
	else if (std::string_view{ imagePathStr }.substr(dotPos) != kPngExtension)
	{
		LOG_ERROR_FMT("Target image filename '{}' was not specified as a .png "
			"- reformatting", imagePathStr);

		imagePathStr.replace(dotPos, imagePathStr.size() - dotPos, kPngExtension);
	}

	return imagePathStr;
}

void FlipPixelsVertically(SDL_Surface* surface, int height)
{
	if (!surface)
	{
		return;
	}

	Uint8* pixels = static_cast<Uint8*>(surface->pixels);
	int pitch = surface->pitch;
	Uint8* temp = new Uint8[pitch];

	for (int i = 0; i < height / 2; ++i)
	{
		Uint8* top = pixels + i * pitch;
		Uint8* bottom = pixels + (height - i - 1) * pitch;
		memcpy(temp, top, pitch);
		memcpy(top, bottom, pitch);
		memcpy(bottom, temp, pitch);
	}

	delete[] temp;
}

} // unnamed

Result<Void> WriteFrameBufferToPNG(SDL_Renderer* renderer, std::string_view fileName)
{
	if (!renderer)
	{
		return MAKE_ERROR("Renderer was null");
	}
	if (fileName.empty())
	{
		return MAKE_ERROR("Target image filename was empty");
	}

	int w = 0, h = 0;
	SDL_GetRendererOutputSize(renderer, &w, &h);

	SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32,
														  SDL_PIXELFORMAT_RGBA32);
	if (!surface)
	{
		return MAKE_ERROR(SDL_GetError());
	}

	if (SDL_RenderReadPixels(renderer, nullptr, SDL_PIXELFORMAT_RGBA32,
		surface->pixels, surface->pitch) != 0)
	{
		SDL_FreeSurface(surface);
		return MAKE_ERROR(SDL_GetError());
	}

	//FlipPixelsVertically(surface, h);

	std::string imageFilepathStr = MakeTestImageFilepathString(fileName);

	if (IMG_SavePNG(surface, imageFilepathStr.c_str()) != 0)
	{
		return MAKE_ERROR(IMG_GetError());
	}

	SDL_FreeSurface(surface);
	LOG_DEBUG_FMT("Image written to path: '{}'", imageFilepathStr);

	return Void{};
}



} // test