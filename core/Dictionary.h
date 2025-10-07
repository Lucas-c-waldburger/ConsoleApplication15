#pragma once
#include <map>
#include <unordered_map>
#include <string>
#include <string_view>
#include <stdexcept>

struct DictionaryHash 
{
    using is_transparent = void;

    template <typename T>
    size_t operator()(T&& t) const noexcept {
        return std::hash<std::string_view>{}(std::string_view(t));
    }
};

struct DictionaryEq {
    using is_transparent = void;

    template <typename L, typename R>
    bool operator()(L&& lhs, R&& rhs) const 
    {
        return std::string_view(lhs) == std::string_view(rhs);
    }
};

template <template <typename, typename, typename...> class MapType, typename Value, typename...Ts>
class DictionaryTemplate : public MapType<std::string, Value, Ts...>
{
private:
    using Super = MapType<std::string, Value, Ts...>;

public:
    Value& operator[](std::string_view sv)
    {
        return Super::operator[](std::string{ sv });
    }

    const Value& operator[](std::string_view sv) const
    {
        auto it = Super::find(sv);
        if (it == Super::end())
        {
            throw std::out_of_range("Key not found");
        }

        return it->second;
    }

    Value& at(std::string_view sv)
    {
        auto it = Super::find(sv);
        if (it == Super::end()) 
        {
            throw std::out_of_range("Key not found");
        }

        return it->second;
    }

    size_t erase(std::string_view sv)
    {
        return Super::erase(std::string{ sv });
    }

    typename Super::iterator erase(typename Super::iterator pos)
    {
        return Super::erase(pos);
    }

    typename Super::iterator erase(typename Super::const_iterator pos)
    {
        return Super::erase(pos);
    }
};


template <typename Value>
using Dictionary = DictionaryTemplate<std::map, Value, std::less<>>;

template <typename Value>
using UnorderedDictionary = 
    DictionaryTemplate<std::unordered_map, Value, DictionaryHash, DictionaryEq>;