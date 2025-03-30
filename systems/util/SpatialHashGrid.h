#pragma once
#include <unordered_map>
#include "QuadTree.h"

struct GridHash 
{
    static constexpr float kCellSize = 64.0f; // Adjust for your game

    static std::pair<int, int> Hash(const AABB& aabb) 
    {
        return { static_cast<int>(aabb.x / kCellSize), 
                 static_cast<int>(aabb.y / kCellSize) };
    }
};

class SpatialHashGrid 
{
    std::unordered_map<std::pair<int, int>, std::vector<AABB>> grid;

public:
    void Insert(const AABB& obj) 
    {
        auto cell = GridHash::Hash(obj);

        grid[cell].push_back(obj);
    }

    void Query(const AABB& area, std::vector<AABB>& results) 
    {
        auto minCell = GridHash::Hash({ area.x, area.y, 0, 0 });
        auto maxCell = GridHash::Hash({ area.x + area.w, area.y + area.h, 0, 0 });

        for (int x = minCell.first; x <= maxCell.first; ++x) 
        {
            for (int y = minCell.second; y <= maxCell.second; ++y) 
            {
                auto iter = grid.find({ x, y });
                if (iter != grid.end()) 
                {
                    for (const auto& obj : iter->second) 
                    {
                        if (area.Intersects(obj)) 
                        {
                            results.push_back(obj);
                        }
                    }
                }
            }
        }
    }
};