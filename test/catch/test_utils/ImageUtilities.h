#pragma once
#include "../CatchUtils.h"
#include "../../../core/commonObjects.h"
#include <SDL.h>
#include "../../../core/Result.h"

namespace test {

constexpr std::string_view kTestImagesDirPath = "test/catch/test_images";
constexpr const char* kPngExtension = ".png";

Result<Void> WriteFrameBufferToPNG(SDL_Renderer* renderer, std::string_view fileName);





} // test