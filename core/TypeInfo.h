#pragma once
#include <iterator>
#include <string_view>

#if defined(__clang__)
#define STAG_PRETTY_FUNCTION __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#define STAG_PRETTY_FUNCTION __FUNCSIG__
#elif defined(__GNUC__)
#define STAG_PRETTY_FUNCTION __PRETTY_FUNCTION__
#else
#error Unsupported compiler
#endif

template <typename T>
inline consteval std::string_view GetTypeName()
{
#if defined(__clang__)

    constexpr std::string_view fn = STAG_PRETTY_FUNCTION;

    constexpr auto marker = std::string_view{ "[T = " };
    constexpr auto markerPos = fn.find(beginMarker);

    static_assert(markerPos != std::string_view::npos,
        "Unable to find type name in compiler function signature");

    constexpr auto begin = markerPos + marker.size();
    constexpr auto end = fn.find(']', begin);

    static_assert(end != std::string_view::npos,
        "Unable to find end of type name in compiler function signature");

    return fn.substr(begin, end - begin);

#elif defined(__GNUC__)

    constexpr std::string_view fn = STAG_PRETTY_FUNCTION;

    constexpr auto marker = std::string_view{ "[with T = " };
    constexpr auto markerPos = fn.find(marker);

    static_assert(markerPos != std::string_view::npos,
        "Unable to find type name in compiler function signature");

    constexpr auto begin = markerPos + marker.size();
    constexpr auto end = fn.find(';', begin);

    static_assert(end != std::string_view::npos,
        "Unable to find end of type name in compiler function signature");

    return fn.substr(begin, end - begin);

#elif defined(_MSC_VER)

    constexpr std::string_view fn = STAG_PRETTY_FUNCTION;

    constexpr auto marker = std::string_view{ "GetTypeName<" };
    constexpr auto markerPos = fn.find(marker);

    static_assert(markerPos != std::string_view::npos,
        "Unable to find type name in compiler function signature");

    constexpr auto begin = markerPos + marker.size();
    constexpr auto end = fn.find('>', begin);

    static_assert(end != std::string_view::npos,
        "Unable to find end of type name in compiler function signature");

    return fn.substr(begin, end - begin);

#else
#error Unsupported compiler
#endif
}

template <typename T>
inline consteval uint64_t GetTypeHash()
{
    constexpr auto typeName = GetTypeName<T>();

    uint64_t hash = 14695981039346656037ull;

    for (const unsigned char c : typeName)
    {
        hash ^= c;
        hash *= 1099511628211ull;
    }

    return hash;
}

template <typename T>
struct TypeInfo
{
    static constexpr std::string_view name = GetTypeName<T>();
    static constexpr uint64_t hash = GetTypeHash<T>();
};

