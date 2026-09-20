#include "TileGridRenderer.h"

#if IMGUI_ENABLED
#include "../../camera/Camera.h"
#include <SDL.h>

namespace ui {

void TileGridRenderer::Update(float)
{
    //if (!camera_)
    //{
    //    return;
    //}

    //const auto camPos = camera_->GetPosition();
    //const auto zoom = camera_->GetZoomScale();
    //const auto bounds = camera_->WorldToScreen<SDL_FRect>(camera_->GetViewport().GetBoundingBox());

    //const float cellSize = tileSize_ * zoom;

    //const int firstTileX = static_cast<int>(std::floor(camPos.x / tileSize_));
    //const int firstTileY = static_cast<int>(std::floor(camPos.y / tileSize_));

    //const int lastTileX = static_cast<int>(std::ceil((camPos.x + vpSize.w / zoom) / tileSize_));
    //const int lastTileY = static_cast<int>(std::ceil((camPos.y + vpSize.h / zoom) / tileSize_));

    //for (int x = firstTileX; x <= lastTileX; ++x)
    //{
    //    const float screenX =
    //        canvasMin_.x +
    //        (x * tileSize_ - camera_.x) * zoom_;

    //    drawList->AddLine(
    //        { screenX, canvasMin_.y },
    //        { screenX, canvasMax_.y },
    //        gridColor);
    //}

    //for (int y = firstTileY; y <= lastTileY; ++y)
    //{
    //    const float screenY =
    //        canvasMin_.y +
    //        (y * tileSize_ - camera_.y) * zoom_;

    //    drawList->AddLine(
    //        { canvasMin_.x, screenY },
    //        { canvasMax_.x, screenY },
    //        gridColor);
    //}
}

} // ui

#endif
