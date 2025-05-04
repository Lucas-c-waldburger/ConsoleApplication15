#include "Mouse.h"

Mouse::Snapshot Mouse::GetSnapshot()
{
	Snapshot mouseSnapshot{};

	mouseSnapshot.activeCursor = GetActiveCursor();
	mouseSnapshot.buttonMask = SDL_GetMouseState(&mouseSnapshot.position.x,
												 &mouseSnapshot.position.y);

	return mouseSnapshot;
}

Handle<MouseCursor> Mouse::AddSystemCursor(SDL_SystemCursor systemCursor)
{
	return AddCursorImpl(systemCursor);
}

Handle<MouseCursor> Mouse::AddColorCursor(const std::string& bmpFileName, SDL_Point clickOffset)
{
	return AddCursorImpl(bmpFileName, clickOffset);
}

bool Mouse::SetActiveCursor(const Handle<MouseCursor>& cursorHandle)
{
	auto it = Mouse::cursors.find(cursorHandle);
	if (it == Mouse::cursors.end())
	{
		return false;
	}

	return it->second.SetAsActiveCursor();
}

Handle<MouseCursor> Mouse::GetActiveCursor()
{
	for (const auto& [handle, mouseCursor] : Mouse::cursors)
	{
		if (mouseCursor.IsActiveCursor())
		{
			return handle;
		}
	}

	return {};
}

bool Mouse::RemoveCursor(const Handle<MouseCursor>& cursorHandle)
{
	return static_cast<bool>(Mouse::cursors.erase(cursorHandle));
}



