#pragma once
#include <iostream>
#include <cassert>

template <typename...> struct TypeList {};

using Entity_t = uint32_t;

static constexpr Entity_t kMaxEntities = 500;
static constexpr Entity_t kInvalidEntity = -1;
static constexpr size_t kMaxComponents = 64;
static constexpr size_t kInvalidIndex = kMaxEntities + 1;

using ComponentSignature = uint64_t;

struct ComponentBitFactory
{
    template <typename T> friend struct BaseComponent;
private:
    static uint64_t GetNextComponentBit()
    {
        static uint64_t bitCounter = 1;
        uint64_t bit = bitCounter;
        bitCounter <<= 1;

        return bit;
    }
};

template <typename Derived>
struct BaseComponent
{
    static inline const uint64_t componentBit = ComponentBitFactory::GetNextComponentBit();
};

template <typename T>
concept ComponentType = std::same_as<std::remove_cvref_t<decltype(T::componentBit)>, uint64_t>;

template <typename T>
struct Exclude
{
    using WrappedType = std::remove_cvref_t<T>;
};
template <typename T> struct is_exclude : std::false_type {};
template <typename T> struct is_exclude<Exclude<T>> : std::true_type {};

template <typename T>
concept ComponentOrExclusionWrappedType = (ComponentType<T> ||
    (is_exclude<T>::value && ComponentType<typename T::WrappedType>));

template <typename Base, ComponentType...Ts> requires (std::derived_from<Ts, Base> && ...)
struct UpcastTo
{
    static_assert(sizeof...(Ts) > 0);
    using WrappedDerivedTypes = TypeList<Ts...>;
    using WrappedBaseType = Base;
};

template <typename T> struct is_upcast_to_wrapper : std::false_type {};
template <typename T, typename...Ts> struct is_upcast_to_wrapper<UpcastTo<T, Ts...>> : std::true_type {};

template <ComponentType...Ts>
struct AnyOf
{
    using WrappedTypes = TypeList<Ts...>;
};

template <typename T> struct is_any_of_wrapper : std::false_type {};
template <typename...Ts> struct is_any_of_wrapper<AnyOf<Ts...>> : std::true_type {};


template <typename Tup, typename Fn, size_t... Is>
static void ForEachInTupleImpl(Tup&& tup, Fn&& fn, std::index_sequence<Is...>)
{
    (fn(std::get<Is>(std::forward<Tup>(tup))), ...);
}

template <typename Tup, typename Fn>
static void ForEachInTuple(Tup&& tup, Fn&& fn)
{
    ForEachInTupleImpl(std::forward<Tup>(tup), std::forward<Fn>(fn),
        std::make_index_sequence<std::tuple_size<std::remove_cvref_t<Tup>>::value>{});
}

template <typename T>
class Handle
{
public:
    Handle() : value_(kInvalid), gen_(kInvalid) {}

    static Handle Create() { return Handle{ handleCounter++, genCounter }; }
    static Handle MakeEmpty() { return Handle{}; }

    bool IsValid() const { return value_ != kInvalid && gen_ == genCounter; }
    size_t GetHash() const noexcept { return value_ * 31 + gen_; }

    auto operator<=>(const Handle&) const = default;

    static void NextGen() { ++genCounter; }

private:
    static int handleCounter;
    static int genCounter;
    static constexpr int kInvalid = -1; 

    Handle(int val, int gen) : value_(val), gen_(gen) {}
    static void ResetAll() { handleCounter = 0; ++genCounter; }

    int value_;
    int gen_;
};

template <typename T> int Handle<T>::handleCounter = 0;
template <typename T> int Handle<T>::genCounter = 0;

namespace std {
    template <typename T>
    struct hash<Handle<T>> {
        size_t operator()(const Handle<T>& handle) const noexcept {
            return handle.GetHash();
        }
    };
}

template <typename T>
static constexpr Handle<T> kEmptyHandle = Handle<T>::MakeEmpty();

template <typename T> requires std::is_arithmetic_v<T>
struct Dimensions
{
    T w = static_cast<T>(0);
    T h = static_cast<T>(0);
};




static constexpr int GetNextPowerOfTwo(int x)
{
    if (x <= 0) return 1;
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;

    return x + 1;
}