//#pragma once
//#include <vector>
//#include <unordered_map>
//#include "../../ecs/EntityT.h"
//
//class UnorderedEntityIdSet
//{
//public:
//    using CIter = typename std::vector<Entity_t>::const_iterator;
//    CIter begin() { return entityIds_.begin(); }
//    CIter end() { return entityIds_.end(); }
//
//    bool Insert(const Entity_t entityId);
//    bool Erase(const Entity_t entityToErase);
//    size_t Size() const;
//    Entity_t operator[](const size_t idx) const;
//
//private:
//    std::vector<Entity_t> entityIds_;
//    std::unordered_map<Entity_t, size_t> entityToIndex_;
//};