//#pragma once
//#include <unordered_map>
//#include <unordered_set>
//#include "QuadTree.h"
//
//class SpatialHashGrid 
//{
//public:
//    float cellSize = 64.0f;
//    std::unordered_map<int64_t, UnorderedEntityIdSet> cells;
//
//    void Clear() 
//    {
//        cells.clear();
//    }
//
//    void Insert(const Entity_t entityId, const AABB& bounds);
//
//    std::vector<EntityColliderBounds> GetIntersecting(const Entity_t entityId, const AABB& bounds,
//                                                      uint8_t flagsFilter = 0x00);
//
//private:
//    static constexpr int64_t Hash(int x, int y) 
//    {
//        return (static_cast<int64_t>(x) << 32) | static_cast<uint32_t>(y);
//    }
//};
//
