#pragma once
#include <limits>
#include <type_traits>

// FIXED STRING
template <size_t N>
struct FixedString 
{
    char value[N];

    // Construct from string literal
    consteval FixedString(char const (&str)[N]) 
    {
        for (size_t i = 0; i < N; ++i)
        {
            value[i] = str[i];
        }
    }

    consteval bool operator==(FixedString const&) const = default;

    constexpr std::size_t size() const { return N; }
    constexpr operator const char* () const { return value; }
};

namespace detail {

template <size_t Idx, FixedString Str, FixedString...Strs>
struct index_of_string 
{ 
    static constexpr size_t value = std::numeric_limits<size_t>::max(); 
};

template <size_t Idx, FixedString Str, FixedString Head, FixedString...Rest>
struct index_of_string<Idx, Str, Head, Rest...>
{
    static constexpr size_t value = (Str == Head)
        ? Idx
        : index_of_string<Idx + 1, Str, Rest...>::value;
};

template <FixedString, FixedString...>
struct contains_string;

template <FixedString Query>
struct contains_string<Query> : std::false_type {};

template <FixedString Query, FixedString Head, FixedString...Tail>
struct contains_string<Query, Head, Tail...> : std::conditional_t<
    (Query == Head),
    std::true_type,
    contains_string<Query, Tail...>>
{};

template <FixedString...>
struct all_strings_unique;

template <>
struct all_strings_unique<> : std::true_type {};

template <FixedString Str>
struct all_strings_unique<Str> : std::true_type {};

template <FixedString Head, FixedString...Tail>
struct all_strings_unique<Head, Tail...> : std::conditional_t<
    contains_string<Head, Tail...>::value,
    std::false_type,
    all_strings_unique<Tail...>>
{};

} // detail

template <FixedString Str, FixedString...Strs>
inline constexpr size_t index_of_string_v = 
    detail::index_of_string<0, Str, Strs...>::value;

template <FixedString Query, FixedString...Rest>
inline constexpr bool contains_string_v = 
    detail::contains_string<Query, Rest...>::value;

template <FixedString...Strs>
inline constexpr bool all_strings_unique_v = 
    detail::all_strings_unique<Strs...>::value;