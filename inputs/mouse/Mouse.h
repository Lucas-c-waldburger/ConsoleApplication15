#pragma once
#include <array>
#include "MouseCursor.h"

class Mouse
{
public:
	struct Snapshot
	{
		Handle<MouseCursor> activeCursor;
		SDL_Point position = { 0, 0 };
		uint32_t buttonMask = 0;
	};

	static Snapshot GetSnapshot();

	static Handle<MouseCursor> AddSystemCursor(SDL_SystemCursor systemCursor);
	static Handle<MouseCursor> AddColorCursor(const std::string& bmpFileName, SDL_Point clickOffset);

	static bool SetActiveCursor(const Handle<MouseCursor>& cursorHandle);
	static Handle<MouseCursor> GetActiveCursor();

	static bool RemoveCursor(const Handle<MouseCursor>& cursorHandle);

private:
	template <typename...Ts>
	static Handle<MouseCursor> AddCursorImpl(Ts&&...args);

	static std::unordered_map<Handle<MouseCursor>, MouseCursor> cursors;
};

std::unordered_map<Handle<MouseCursor>, MouseCursor> Mouse::cursors{};

template<typename ...Ts>
inline Handle<MouseCursor> Mouse::AddCursorImpl(Ts && ...args)
{
	auto result = MouseCursor::Create(std::forward<Ts>(args)...);
	if (!result.Success())
	{
		LOG_ERROR(result.GetError());
		return {};
	}

	auto handle = result.GetValue().GetHandle();
	assert(handle.IsValid());

	bool inserted = Mouse::cursors.emplace(handle, std::move(result.GetValue())).second;
	assert(inserted);

	return handle;
}
