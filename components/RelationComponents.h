#pragma once
#include "BaseComponent.h"
#include "../ecs/EntityT.h"
#include "ComponentConcepts.h"
#include <unordered_map>

// slave signature uses componentBits to define the kinds of 
// relations they have with the parent, meaning that a 
// signature of "Collider::componentBit" communicates that this child
// should have the parent should consider it as its own component for logic/operations
//using SlaveSignature = uint64_t;

//struct Child
//{
//    Entity_t entityId = kInvalidEntity;
//    SlaveSignature slaveSignature = 0;
//    bool operator==(const Child& rhs) const {
//        return entityId == rhs.entityId;
//    }
//};

//namespace std {
//    template <>
//    struct hash<Child> {
//        size_t operator()(const Child& ch) const noexcept {
//            return std::hash<Entity_t>{}(ch.entityId);
//        }
//    };
//}

struct Parent : BaseComponent<Parent, 4>
{
    Entity_t entityId = kInvalidEntity;
};

struct Children : BaseComponent<Children, 5>
{
    std::unordered_set<Entity_t> childEntityIds;
};

template <typename T>
concept RelationalComponentType = (std::same_as<T, Parent> || std::same_as<T, Children>);