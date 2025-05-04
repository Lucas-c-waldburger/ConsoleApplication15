#pragma once
#include "../../core/Result.h"
#include "../../core/Handle.h"
#include "../../sdl/SDLUtils.h"

class MouseCursor
{
public:
	MouseCursor() = default;
	~MouseCursor() = default;

	MouseCursor(MouseCursor&) = delete;
	MouseCursor& operator=(MouseCursor&) = delete;

	MouseCursor(MouseCursor&& rhs) noexcept;
	MouseCursor& operator=(MouseCursor&& rhs) noexcept;

	bool IsActiveCursor() const;
	bool SetAsActiveCursor();
	const Handle<MouseCursor>& GetHandle() const;

	static Result<MouseCursor> Create(SDL_SystemCursor systemCursor);
	static Result<MouseCursor> Create(const std::string& bmpFileName, SDL_Point clickOffset);

private:
	Handle<MouseCursor> handle_;
	UniqueCursorPtr cursor_;
	UniqueSurfacePtr surface_;
};