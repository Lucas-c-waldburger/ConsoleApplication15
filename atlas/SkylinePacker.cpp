#include "SkylinePacker.h"
#include <iostream>

std::optional<SDL_Point> SkylinePacker::Insert(int w, int h)
{
    int bestIndex = -1;
    int bestY = std::numeric_limits<int>::max();
    int bestX = 0;

    // Find the best position to fit the rectangle
    for (size_t i = 0; i < skyline_.size(); i++) {
        int y = CanFitAt(i, w, h);
        if (y != -1 && y < bestY) {
            bestIndex = static_cast<int>(i);
            bestY = y;
            bestX = skyline_[i].x;
        }
    }

    if (bestIndex == -1)
    {
        std::cerr << "No space available for rect\n";

        return std::nullopt;
    }

    // Place the rectangle
    UpdateSkyline(bestIndex, bestX, bestY, w, h);

    return SDL_Point{ bestX, bestY };
}

int SkylinePacker::CanFitAt(int index, int w, int h)
{
    int x = skyline_[index].x;
    int y = skyline_[index].y;
    int widthLeft = w;

    for (size_t i = index; i < skyline_.size(); i++)
    {
        if (skyline_[i].y > y)
        {
            return -1; // Blocked by a higher segment
        }

        widthLeft -= skyline_[i].w;

        if (widthLeft <= 0)
        {
            return y; // Found enough space
        }
    }

    return -1; // No fit
}

void SkylinePacker::UpdateSkyline(int index, int x, int y, int w, int h)
{
    Node newNode = { x, y + h, w };

    // Remove covered parts of the skyline
    int i = index;

    while (i < skyline_.size() && skyline_[i].x < x + w)
    {
        if (skyline_[i].x + skyline_[i].w > x + w)
        {
            skyline_[i].x = x + w;
            skyline_[i].w -= w;

            break;
        }
        else
        {
            skyline_.erase(skyline_.begin() + i);
        }
    }

    skyline_.insert(skyline_.begin() + index, newNode);

    MergeAdjacentNodes();
}

void SkylinePacker::MergeAdjacentNodes()
{
    for (size_t i = 0; i < skyline_.size() - 1; i++)
    {
        if (skyline_[i].y == skyline_[i + 1].y)
        {
            skyline_[i].w += skyline_[i + 1].w;
            skyline_.erase(skyline_.begin() + i + 1);

            i--;
        }
    }
}