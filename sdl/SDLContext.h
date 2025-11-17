#pragma once
#include <SDL.h>
#include <memory>
#include "../core/Result.h"

namespace sdl {

class Window
{
public:
	struct Args
	{
		int x = SDL_WINDOWPOS_CENTERED;
		int y = SDL_WINDOWPOS_CENTERED;
		int w = 1200;
		int h = 900;
		std::string title;
		Uint32 flags = (
			SDL_WINDOW_SHOWN | 
			SDL_WINDOW_RESIZABLE | 
			SDL_WINDOW_ALLOW_HIGHDPI
		);
	};

	Window() = default;

	static Result<Window> Create(Args&& args);

private:
	using Ptr = std::unique_ptr<SDL_Window, 
		decltype([](SDL_Window* w) { SDL_DestroyWindow(w); })>;
	Ptr ptr_;
};














} // sdl

