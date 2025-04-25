#pragma once
#include <box2d/box2d.h>
#include "../core/Handle.h"

class B2Shape;
class B2Body;

template <typename T>
concept B2IdType = (std::same_as<T, b2BodyId> || std::same_as<T, b2ShapeId>);

template <typename T>
concept B2HandleAliasType = (std::same_as<T, B2Body> || std::same_as<T, B2Shape>);

template <B2IdType T>
struct B2BodyShapeIdEq
{
    bool operator()(const T& lhs, const T& rhs) const {
        return lhs.index1 == rhs.index1 &&
               lhs.world0 == rhs.world0 &&
               lhs.generation == rhs.generation;
    }
};

inline void HashCombine(std::size_t& seed, std::size_t value)
{
    seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template <B2IdType T>
struct B2BodyShapeIdHash
{
    size_t operator()(const T& id) const noexcept
    {
        std::size_t hash = 0;
        HashCombine(hash, std::hash<int32_t>{}(id.index1));
        HashCombine(hash, std::hash<uint16_t>{}(id.world0));
        HashCombine(hash, std::hash<uint16_t>{}(id.generation));

        return hash;
    }
};

template <> 
class Handle<B2Body>
{
public:
    template <typename...HandleTs>
    friend class HandleFactory;

    using B2IdType = b2BodyId;

    Handle() = default;
    bool operator==(const Handle& rhs) const { return B2BodyShapeIdEq<b2BodyId>{}(bodyId_, rhs.bodyId_); }
    size_t GetHash() const noexcept { return B2BodyShapeIdHash<b2BodyId>{}(bodyId_); }
    bool IsValid() const { return b2Body_IsValid(bodyId_); }
    operator const b2BodyId& () const { return bodyId_; }

    friend std::ostream& operator<<(std::ostream& os, const Handle& handle)
    {
        if (!handle.IsValid()) { os << "{ INVALID }"; }
        else { os << "{ VALID }"; }
        return os;
    }

private:
    Handle(b2BodyId bodyId) : bodyId_(bodyId) {}

    b2BodyId bodyId_ = b2_nullBodyId;
};

template <>
class Handle<B2Shape>
{
public:
public:
    template <typename...HandleTs>
    friend class HandleFactory;

    using B2IdType = b2ShapeId;

    Handle() = default;
    bool operator==(const Handle& rhs) const { return B2BodyShapeIdEq<b2ShapeId>{}(shapeId_, rhs.shapeId_); }
    size_t GetHash() const noexcept { return B2BodyShapeIdHash<b2ShapeId>{}(shapeId_); }
    bool IsValid() const { return b2Shape_IsValid(shapeId_); }
    operator const b2ShapeId& () const { return shapeId_; }

    friend std::ostream& operator<<(std::ostream& os, const Handle& handle)
    {
        if (!handle.IsValid()) { os << "{ INVALID }"; }
        else { os << "{ VALID }"; }
        return os;
    }

private:
    Handle(b2ShapeId shapeId) : shapeId_(shapeId) {}

    b2ShapeId shapeId_ = b2_nullShapeId;
};

namespace std {
    template <>
    struct hash<Handle<B2Body>> {
        size_t operator()(const Handle<B2Body>& handle) const noexcept {
            return handle.GetHash();
        }
    };
    template <>
    struct hash<Handle<B2Shape>> {
        size_t operator()(const Handle<B2Shape>& handle) const noexcept {
            return handle.GetHash();
        }
    };
}

