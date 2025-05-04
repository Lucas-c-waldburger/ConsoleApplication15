#include "MouseCursor.h"
#include "../../sdl/SDLUtils.h"

MouseCursor::MouseCursor(MouseCursor&& rhs) noexcept : 
	cursor_(std::move(rhs.cursor_)), surface_(std::move(rhs.surface_)) {}

MouseCursor& MouseCursor::operator=(MouseCursor&& rhs) noexcept
{
	if (this != &rhs)
	{
		cursor_ = std::move(rhs.cursor_);
		surface_ = std::move(rhs.surface_);
	}

	return *this;
}

bool MouseCursor::IsActiveCursor() const
{
	if (!cursor_)
	{
		return false;
	}

	auto* active = SDL_GetCursor();
	if (!active)
	{
		return false;
	}

	return active == cursor_.get();
}

bool MouseCursor::SetAsActiveCursor()
{
	if (IsActiveCursor())
	{
		return true;
	}
	if (cursor_)
	{
		SDL_SetCursor(cursor_.get());
		return true;
	}

	return false;
}

const Handle<MouseCursor>& MouseCursor::GetHandle() const
{
	return handle_;
}

Result<MouseCursor> MouseCursor::Create(SDL_SystemCursor systemCursor)
{
	MouseCursor mouseCursor{};

	mouseCursor.cursor_ = MakeUniqueCursor(systemCursor);
	if (!mouseCursor.cursor_)
	{
		return MAKE_ERROR("SDL_CreateSystemCursor failed to create cursor");
	}

	mouseCursor.handle_ = Handle<MouseCursor>::Create();
	assert(mouseCursor.handle_.IsValid());

	return Result<MouseCursor>{std::move(mouseCursor)};
}

Result<MouseCursor> MouseCursor::Create(const std::string& bmpFileName, SDL_Point clickOffset)
{
	MouseCursor mouseCursor{};

	mouseCursor.surface_ = MakeUniqueSurfacePtrBMP(bmpFileName);
	if (!mouseCursor.surface_)
	{
		return MAKE_ERROR("SDL_LoadBMP failed to create surface");
	}

	mouseCursor.cursor_ = MakeUniqueCursor(mouseCursor.surface_, clickOffset);
	if (!mouseCursor.cursor_)
	{
		return MAKE_ERROR("SDL_CreateColorCursor failed to create cursor");
	}

	mouseCursor.handle_ = Handle<MouseCursor>::Create();
	assert(mouseCursor.handle_.IsValid());

	return Result<MouseCursor>{std::move(mouseCursor)};
}
