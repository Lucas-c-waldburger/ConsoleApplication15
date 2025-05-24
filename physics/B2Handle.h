#pragma once
#include <box2d/box2d.h>
#include "../core/Handle.h"

inline constexpr bool operator==(const b2WorldId& lhs, const b2WorldId& rhs)
{
    return lhs.generation == rhs.generation && lhs.index1 == rhs.index1;
}

class B2Shape;
class B2Body;
class B2Joint;

template <typename T>
concept B2IdType = (std::same_as<T, b2BodyId> || std::same_as<T, b2ShapeId> || std::same_as<T, b2JointId>);

template <typename T>
concept B2HandleAliasType = (std::same_as<T, B2Body> || std::same_as<T, B2Shape> || std::same_as<T, B2Joint>);

template <B2IdType T>
inline constexpr bool operator==(const T& lhs, const T& rhs)
{
    return lhs.index1 == rhs.index1 &&
           lhs.world0 == rhs.world0 &&
           lhs.generation == rhs.generation;
}

template <B2IdType T>
struct B2BodyShapeJointIdEq
{
    bool operator()(const T& lhs, const T& rhs) const {
        return lhs.index1 == rhs.index1 &&
               lhs.world0 == rhs.world0 &&
               lhs.generation == rhs.generation;
    }
};

template <B2IdType T>
struct B2BodyShapeJointIdHash
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

// TODO: can use one template for all these i think
template <> 
class Handle<B2Body> : public HandleBase<Handle<B2Body>>
{
public:
    friend class HandleBase<Handle<B2Body>>;
    using B2IdType = b2BodyId;

    Handle() = default;
    bool operator==(const Handle& rhs) const { return bodyId_ == rhs.bodyId_; }
    bool operator==(const b2BodyId& bodyId) const { return bodyId_ == bodyId; }

    operator const b2BodyId& () const { return bodyId_; }

private:
    size_t GetHashImpl() const noexcept { return B2BodyShapeJointIdHash<b2BodyId>{}(bodyId_); }
    bool IsValidImpl() const { return b2Body_IsValid(bodyId_); }

    static Handle CreateImpl(b2BodyId bodyId) { return Handle{ bodyId }; }

    Handle(b2BodyId bodyId) : bodyId_(bodyId) {}

    b2BodyId bodyId_ = b2_nullBodyId;
};

template <>
class Handle<B2Shape> : public HandleBase<Handle<B2Shape>>
{
public:
    friend class HandleBase<Handle<B2Shape>>;
    using B2IdType = b2ShapeId;

    Handle() = default;
    bool operator==(const Handle& rhs) const { return shapeId_ == rhs.shapeId_; }
    bool operator==(const b2ShapeId& shapeId) const { return shapeId_ == shapeId; }

    operator const b2ShapeId& () const { return shapeId_; }

private:
    size_t GetHashImpl() const noexcept { return B2BodyShapeJointIdHash<b2ShapeId>{}(shapeId_); }
    bool IsValidImpl() const { return b2Shape_IsValid(shapeId_); }

    static Handle CreateImpl(b2ShapeId shapeId) { return Handle{ shapeId }; }

    Handle(b2ShapeId shapeId) : shapeId_(shapeId) {}

    b2ShapeId shapeId_ = b2_nullShapeId;
};

template <>
class Handle<B2Joint> : public HandleBase<Handle<B2Joint>>
{
public:
    friend class HandleBase<Handle<B2Joint>>;
    using B2IdType = b2JointId;

    Handle() = default;
    bool operator==(const Handle& rhs) const { return jointId_ == rhs.jointId_; }
    bool operator==(const b2JointId& jointId) const { return jointId_ == jointId; }

    operator const b2JointId& () const { return jointId_; }

private:
    size_t GetHashImpl() const noexcept { return B2BodyShapeJointIdHash<b2JointId>{}(jointId_); }
    bool IsValidImpl() const { return b2Joint_IsValid(jointId_); }

    static Handle CreateImpl(b2JointId jointId) { return Handle{ jointId }; }

private:
    Handle(b2JointId jointId) : jointId_(jointId) {}

    b2JointId jointId_ = b2_nullJointId;
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
    template <>
    struct hash<Handle<B2Joint>> {
        size_t operator()(const Handle<B2Joint>& handle) const noexcept {
            return handle.GetHash();
        }
    };
}

