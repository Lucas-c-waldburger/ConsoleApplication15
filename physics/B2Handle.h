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
class B2Chain;

template <typename T>
concept B2IdType = (std::same_as<T, b2BodyId> || std::same_as<T, b2ShapeId> ||
                    std::same_as<T, b2JointId> || std::same_as<T, b2ChainId>);

template <typename T>
concept B2HandleAliasType = (std::same_as<T, B2Body> || std::same_as<T, B2Shape> || 
                             std::same_as<T, B2Joint> || std::same_as<T, B2Chain>);

template <B2HandleAliasType T>
struct B2IdTypeForB2ApiType;

template <> struct B2IdTypeForB2ApiType<B2Body>  { using type = b2BodyId; };
template <> struct B2IdTypeForB2ApiType<B2Shape> { using type = b2ShapeId; };
template <> struct B2IdTypeForB2ApiType<B2Joint> { using type = b2JointId; };
template <> struct B2IdTypeForB2ApiType<B2Chain> { using type = b2ChainId; };

template <B2IdType T>
inline bool CheckB2IdValid(const T& id)
{
    if constexpr (std::same_as<T, b2BodyId>)
    {
        return b2Body_IsValid(id);
    }
    else if constexpr (std::same_as<T, b2ShapeId>)
    {
        return b2Shape_IsValid(id);
    }
    else if constexpr (std::same_as<T, b2JointId>)
    {
        return b2Joint_IsValid(id);
    }
    else if constexpr (std::same_as<T, b2ChainId>)
    {
        return b2Chain_IsValid(id);
    }
    else
    {
        return false;
    }
}

template <B2IdType T>
inline constexpr bool operator==(const T& lhs, const T& rhs)
{
    return lhs.index1 == rhs.index1 &&
           lhs.world0 == rhs.world0 &&
           lhs.generation == rhs.generation;
}

template <B2IdType T>
inline constexpr bool operator<(const T& lhs, const T& rhs)
{
    if (lhs.index1 != rhs.index1) { return lhs.index1 < rhs.index1; }
    if (lhs.world0 != rhs.world0) { return lhs.world0 < rhs.world0; }

    return lhs.generation < rhs.generation;
}

template <B2IdType T>
struct B2IdEq
{
    bool operator()(const T& lhs, const T& rhs) const {
        return lhs.index1 == rhs.index1 &&
               lhs.world0 == rhs.world0 &&
               lhs.generation == rhs.generation;
    }
};

template <B2IdType T>
struct B2IdHash
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

template <B2HandleAliasType T>
class Handle<T> : public IHandle<Handle<T>>
{
public:
    friend class IHandle<Handle<T>>;
    using B2IdType = B2IdTypeForB2ApiType<T>::type;

    Handle() = default;
    bool operator==(const Handle& rhs) const { return id_ == rhs.id_; }
    bool operator==(const B2IdType& b2Id) const { return id_ == b2Id; }

    bool operator<(const Handle& rhs) const { return id_ < rhs.id_; }

    operator const B2IdType& () const { return id_; }

private:
    size_t GetHashImpl() const noexcept { return B2IdHash<B2IdType>{}(id_); }
    bool IsValidImpl() const { return CheckB2IdValid(id_); }

    static Handle CreateImpl(B2IdType id) { return Handle{ id }; }

    Handle(B2IdType id) : id_(id) {}

    B2IdType id_ = {};
};

namespace std {
    template <B2HandleAliasType T>
    struct hash<Handle<T>> {
        size_t operator()(const Handle<T>& handle) const noexcept {
            return handle.GetHash();
        }
    };
}

