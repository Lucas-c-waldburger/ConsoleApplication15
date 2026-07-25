#pragma once
#include <vector>
#include <cassert>
#include <ranges>
#include <optional>
#include "SOAConcepts.h"

template <typename Class, auto...MemberPtrs> requires SOACompatible<Class, MemberPtrs...>
class StableSOA
{
protected:
    using MemberPtrTuple = std::tuple<decltype(MemberPtrs)...>;
    using MemberValueTuple = std::tuple<std::vector<
        typename member_ptr_traits<MemberPtrs>::value_type>
        ...>;

    static inline constexpr MemberPtrTuple memberPtrs_{ MemberPtrs... };
    MemberValueTuple memberValues_;

public:
    static inline constexpr size_t MemberCount = sizeof...(MemberPtrs);

    using SliceType = Class;
    using ViewType = std::tuple<
        typename member_ptr_traits<MemberPtrs>::value_type&
    ...>; 
    using ConstViewType = std::tuple<
        const typename member_ptr_traits<MemberPtrs>::value_type&
    ...>;

    //StableSOA() = default;
    //~StableSOA() = default;

    size_t Size() const
    {
        return std::get<0>(memberValues_).size();
    }

    bool Empty() const
    {
        return std::get<0>(memberValues_).empty();
    }

    void Reserve(size_t newSize)
    {
        [&] <std::size_t...Is>(std::index_sequence<Is...>)
        {
            ((std::get<Is>(memberValues_).reserve(newSize)), ...);
        }(std::make_index_sequence<MemberCount>{});
    }

    size_t Capacity() const
    {
        return std::get<0>(memberValues_).capacity();
    }

    size_t PushBack(const Class& obj)
    {
        const size_t newIdx = Size();

        [&] <std::size_t...Is>(std::index_sequence<Is...>)
        {
            ((std::get<Is>(memberValues_)
                .push_back(obj.*std::get<Is>(memberPtrs_))), ...);
        }(std::make_index_sequence<MemberCount>{});

        return newIdx;
    }

    size_t PushBack(Class&& obj)
    {
        const size_t newIdx = Size();

        [&] <std::size_t...Is>(std::index_sequence<Is...>)
        {
            ((std::get<Is>(memberValues_)
                .push_back(std::move(obj.*std::get<Is>(memberPtrs_)))), ...);
        }(std::make_index_sequence<MemberCount>{});

        return newIdx;
    }

    Class MakeSlice(size_t index) const
    {
        if (index >= Size())
        {
            return Class{};
        }

        return[&]<std::size_t...Is>(std::index_sequence<Is...>)
        {
            return Class{ std::get<Is>(memberValues_)[index]... };
        }(std::make_index_sequence<MemberCount>{});
    }

    auto GetView(size_t index)
    {
        assert(index < Size());

        return[&]<std::size_t...Is>(std::index_sequence<Is...>)
        {
            using Ret = std::tuple<
                typename member_ptr_traits<MemberPtrs>::value_type&...
            >;

            return Ret{ std::get<Is>(memberValues_)[index]... };
        }(std::make_index_sequence<MemberCount>{});
    }

    auto GetView(size_t index) const
    {
        assert(index < Size());

        return[&]<std::size_t...Is>(std::index_sequence<Is...>)
        {
            using Ret = std::tuple<
                const typename member_ptr_traits<MemberPtrs>::value_type&...
            >;

            return Ret{ std::get<Is>(memberValues_)[index]... };
        }(std::make_index_sequence<MemberCount>{});
    }

    template <auto...PtrArgs> requires (sizeof...(PtrArgs) > 1)
    auto GetView(size_t index)
    {
        using Ret = std::tuple<
            typename member_ptr_traits<PtrArgs>::value_type&...
        >;

        assert(index < Size());

        return Ret{ (std::get<index_of_member_ptr<PtrArgs, 0, MemberPtrs...>::value>(
            memberValues_)[index]
        )... };
    }

    template <auto...PtrArgs> requires (sizeof...(PtrArgs) > 1)
    auto GetView(size_t index) const
    {
        using Ret = std::tuple<
            const typename member_ptr_traits<PtrArgs>::value_type&...
        >;

        assert(index < Size());

        return Ret{ (std::get<index_of_member_ptr<PtrArgs, 0, MemberPtrs...>::value>(
            memberValues_)[index]
        )... };
    }

    template <auto PtrArg>
    decltype(auto) GetView(size_t index)
    {
        assert(index < Size());

        auto& self = *this;

        return std::get<index_of_member_ptr<PtrArg, 0, MemberPtrs...>::value>(self.memberValues_)[index];
    }

    template <auto PtrArg>
    decltype(auto) GetView(size_t index) const
    {
        assert(index < Size());

        const auto& self = *this;

        return std::get<index_of_member_ptr<PtrArg, 0, MemberPtrs...>::value>(self.memberValues_)[index];
    }

    template <auto...PtrArgs> requires (sizeof...(PtrArgs) > 0)
    auto TryGetView(size_t index)
    {
        using TupleType = std::tuple<
            typename member_ptr_traits<PtrArgs>::value_type&...
        >;
        using OptionalType = std::optional<TupleType>;

        if (index >= Size())
        {
            return OptionalType{ std::nullopt };
        }

        return OptionalType{
            TupleType{
                std::get<index_of_member_ptr<PtrArgs, 0, MemberPtrs...>::value>(
                    memberValues_)[index]...
            }
        };
    }

    template <auto...PtrArgs> requires (sizeof...(PtrArgs) > 0)
    auto TryGetView(size_t index) const
    {
        using TupleType = std::tuple<
            const typename member_ptr_traits<PtrArgs>::value_type&...
        >;
        using OptionalType = std::optional<TupleType>;

        if (index >= Size())
        {
            return OptionalType{ std::nullopt };
        }

        return OptionalType{
            TupleType{
                std::get<index_of_member_ptr<PtrArgs, 0, MemberPtrs...>::value>(
                    memberValues_)[index]...
            }
        };
    }

    auto TryGetView(size_t index)
    { 
        return TryGetView<MemberPtrs...>(index);
    }

    auto TryGetView(size_t index) const
    {
        return TryGetView<MemberPtrs...>(index);
    }

    auto ForEach()
    {
        return std::apply([](auto&...vecs) {
            return std::views::zip(vecs...);
            }, memberValues_);
    }

    auto ForEach() const
    {
        return std::apply([](auto&...vecs) {
            return std::views::zip(vecs...);
            }, memberValues_);
    }

    template <auto PtrArg>
    auto ForEach()
    {
        return std::views::all(
            std::get<index_of_member_ptr<PtrArg, 0, MemberPtrs...>::value>(
                memberValues_));
    }

    template <auto PtrArg>
    auto ForEach() const
    {
        return std::views::all(
            std::get<index_of_member_ptr<PtrArg, 0, MemberPtrs...>::value>(
                memberValues_));
    }

    template <auto...PtrArgs> requires (sizeof...(PtrArgs) > 1)
    auto ForEach() const
    {
        return std::views::zip(
            std::get<index_of_member_ptr<PtrArgs, 0, MemberPtrs...>::value>(
                memberValues_)
            ...);
    }

    template <auto...PtrArgs> requires (sizeof...(PtrArgs) > 1)
    auto ForEach()
    {
        return std::views::zip(
            std::get<index_of_member_ptr<PtrArgs, 0, MemberPtrs...>::value>(
                memberValues_)
            ...);
    }
};