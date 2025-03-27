#pragma once
#include "SDLite.h"
#include <optional>


class SkylinePacker 
{
public:
    SkylinePacker(int width, int height) : atlasWidth_(width), atlasHeight(height) 
    {
        skyline_.push_back({ 0, 0, width }); 
    }

    std::optional<SDL_Point> Insert(int w, int h);

private:
    struct Node { int x, y, w; };

    int atlasWidth_; 
    int atlasHeight;
    std::vector<Node> skyline_;

    int CanFitAt(int index, int w, int h);
    void UpdateSkyline(int index, int x, int y, int w, int h);
    void MergeAdjacentNodes();
};

