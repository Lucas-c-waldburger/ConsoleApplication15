#pragma once
#include "../components/ComponentIncludes.h"
#include "../core/TypeUtils.h"

// components that can't be mutated through Entity API (must use EntityPassKey)
template <typename T>
static constexpr bool public_mutable_component_v = (
    !(RelationalComponentType<T> ||
      std::same_as<T, EntityFlags> ||
      std::same_as<T, ActiveState> ||
      std::same_as<T, ActiveAudio> ||
      std::same_as<T, TextRenderableGlyphCache> ||
      std::same_as<T, MarkedDestroyed> ||
      std::same_as<T, NeedsAnimationUpdate>)
    );

namespace detail {

template <typename TupLike> requires is_tuplike_v<TupLike>
struct all_public_mutable_components : std::false_type {};

template <template <typename...> class TupLike, typename...Ts>
struct all_public_mutable_components<TupLike<Ts...>>
{
    static constexpr bool value = (public_mutable_component_v<Ts> && ...);
};

} // detail

template <typename TupLike> requires is_tuplike_v<TupLike>
static constexpr bool all_public_mutable_components_v =
    detail::all_public_mutable_components<TupLike>::value;