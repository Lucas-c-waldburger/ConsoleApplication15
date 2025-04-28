//#include "UnorderedEntityIdSet.h"
//#include <cassert>
//
//bool UnorderedEntityIdSet::Insert(const Entity_t entityId)
//{
//    if (entityId == kInvalidEntity)
//    {
//        return false;
//    }
//
//    if (entityToIndex_.contains(entityId))
//    {
//        return false;
//    }
//
//    const size_t newIdx = entityIds_.size();
//
//    entityIds_.push_back(entityId);
//    entityToIndex_[entityId] = newIdx;
//
//    return true;
//}
//
//bool UnorderedEntityIdSet::Erase(const Entity_t entityToErase)
//{
//    auto it = entityToIndex_.find(entityToErase);
//    if (it == entityToIndex_.end())
//    {
//        return false;
//    }
//
//    assert(!entityToIndex_.empty());
//
//    const size_t overwriteIdx = it->second;
//    const Entity_t backEntity = entityIds_.back();
//
//    entityIds_[overwriteIdx] = backEntity;
//    entityToIndex_[backEntity] = overwriteIdx;
//
//    entityIds_.pop_back();
//    entityToIndex_.erase(entityToErase);
//
//    return true;
//}
//
//size_t UnorderedEntityIdSet::Size() const
//{
//    assert(entityIds_.size() == entityToIndex_.size());
//    return entityIds_.size();
//}
//
//Entity_t UnorderedEntityIdSet::operator[](const size_t idx) const
//{
//    assert(idx < entityIds_.size());
//    return entityIds_[idx];
//}
