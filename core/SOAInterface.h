#pragma once
#include <ranges>
#include <optional>

template <typename Derived>
concept HasSizeMethod = requires(const Derived& d) {
    { d.Size() } -> std::convertible_to<size_t>;
};

template <typename Derived>
struct SOAInterface
{
    //template <size_t Idx, auto...MemberPtrs>
    //using Slice = std::tuple<decltype((((*static_cast<Derived*>(this)).*MemberPtrs)[Idx]))...>;

    template <auto... MemberPtrs>
    using ViewType = std::tuple<decltype((std::declval<Derived&>().*MemberPtrs)[0])...>;

    template <auto... MemberPtrs>
    using ConstViewType = std::tuple<decltype((std::declval<const Derived&>().*MemberPtrs)[0])...>;

    template <auto...MemberPtrs>
        requires ((std::is_member_object_pointer_v<decltype(MemberPtrs)> && ...))
    auto GetView(size_t index)
    {
        auto* self = static_cast<Derived*>(this);

        return std::tuple<
            decltype((((*self).*MemberPtrs)[index]))...
        >(
            (((*self).*MemberPtrs)[index])...
        );
    }

    template <auto...MemberPtrs>
        requires ((std::is_member_object_pointer_v<decltype(MemberPtrs)> && ...))
    auto GetView(size_t index) const
    {
        const auto* self = static_cast<const Derived*>(this);

        return std::tuple<
            decltype((((*self).*MemberPtrs)[index]))...
        >(
            (((*self).*MemberPtrs)[index])...
        );
    }

    template <auto... MemberPtrs>
    auto TryGetView(size_t index) -> std::optional<ViewType<MemberPtrs...>>
    {
        auto* self = static_cast<Derived*>(this);

        const size_t size = std::get<0>(std::tie(self->*MemberPtrs...)).size();
        if (index >= size)
        {
            return std::nullopt;
        }

        return std::optional<ViewType<MemberPtrs...>>((((*self).*MemberPtrs)[index])...);
    }

    template <auto... MemberPtrs>
    auto TryGetView(size_t index) const -> std::optional<ConstViewType<MemberPtrs...>>
    {
        auto* self = static_cast<const Derived*>(this);

        const size_t size = std::get<0>(std::tie(self->*MemberPtrs...)).size();
        if (index >= size)
        {
            return std::nullopt;
        }

        return ConstViewType<MemberPtrs...>((((*self).*MemberPtrs)[index])...);
    }

    template <auto... MemberPtrs>
    auto ForEach() requires ((std::ranges::viewable_range<
        decltype((std::declval<Derived>().*MemberPtrs))
    > && ...))
    {
        auto& self = static_cast<Derived&>(*this);
        return std::views::zip((self.*MemberPtrs)...);
    }

    template <auto... MemberPtrs>
    auto ForEach() const requires ((std::ranges::viewable_range<
        decltype((std::declval<Derived>().*MemberPtrs))
    > && ...))
    {
        const auto& self = static_cast<const Derived&>(*this);
        return std::views::zip((self.*MemberPtrs)...);
    }
};