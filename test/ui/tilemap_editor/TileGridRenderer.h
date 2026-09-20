#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "../../core/commonObjects.h"
#include <SDL_pixels.h>
#include <imgui.h>

class Camera;

namespace ui {

struct Tile
{
	uint32_t id = 0;
};

class TileMap
{
public:
};

class TileGridRenderer
{
public:
	explicit TileGridRenderer(const Camera& cam) : camera_(&cam) {}

	void Update(float);

private:
	const Camera* camera_ = nullptr;
	float tileSize_ = 32.0f;
	SDL_Color gridColor_ = { 255, 0, 0, 150 };
};



} // ui

#endif