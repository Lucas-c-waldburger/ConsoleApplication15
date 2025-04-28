//#include "SpatialHashGrid.h"
//#include "../../ecs/Ecs.h"
//
//void SpatialHashGrid::Insert(const Entity_t entityId, const AABB& bounds)
//{
//    SDL_FPoint min = { bounds.x, bounds.y };
//    SDL_FPoint max = { bounds.x + bounds.w, bounds.y + bounds.h };
//
//    int minX = static_cast<int>(std::floor(min.x / cellSize));
//    int minY = static_cast<int>(std::floor(min.y / cellSize));
//    int maxX = static_cast<int>(std::floor(max.x / cellSize));
//    int maxY = static_cast<int>(std::floor(max.y / cellSize));
//
//    for (int y = minY; y <= maxY; ++y)
//    {
//        for (int x = minX; x <= maxX; ++x)
//        {
//            int64_t key = Hash(x, y);
//
//            cells[key].Insert(entityId);
//        }
//    }
//}
//
//std::vector<EntityColliderBounds> SpatialHashGrid::GetIntersecting(const Entity_t entityId, const AABB& bounds, uint8_t flagsFilter)
//{
//    SDL_FPoint min = { bounds.x, bounds.y };
//    SDL_FPoint max = { bounds.x + bounds.w, bounds.y + bounds.h };
//
//    int minX = static_cast<int>(std::floor(min.x / cellSize));
//    int minY = static_cast<int>(std::floor(min.y / cellSize));
//    int maxX = static_cast<int>(std::floor(max.x / cellSize));
//    int maxY = static_cast<int>(std::floor(max.y / cellSize));
//
//    std::unordered_set<Entity_t> unique;
//    std::vector<EntityColliderBounds> result;
//
//    for (int y = minY; y <= maxY; ++y)
//    {
//        for (int x = minX; x <= maxX; ++x)
//        {
//            int64_t key = Hash(x, y);
//
//            auto it = cells.find(key);
//            if (it == cells.end())
//            {
//                continue;
//            }
//
//            auto& entityIds = it->second;
//            size_t i = 0;
//            while (i < entityIds.Size())
//            {
//                if (unique.contains(entityIds[i]) || entityIds[i] == entityId)
//                {
//                    ++i;
//                    continue;
//                }
//
//                auto entity = ECS::GetEntityByID(entityIds[i]);
//
//                auto entityBoundsComp = EntityColliderBounds::CreateFromEntity(entity, flagsFilter);
//                if (!entityBoundsComp.has_value())
//                {
//                    assert(entityIds.Erase(entityIds[i]));
//                }
//                else
//                {
//                    if (bounds.Intersects(entityBoundsComp->boundingBox))
//                    {
//                        unique.insert(entityIds[i]);
//                        result.push_back(*entityBoundsComp);
//                    }
//
//                    ++i;
//                }
//            }
//        }
//    }
//
//    return result;
//}
//
//
