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
    using iterator = typename Super::iterator;
    using const_iterator = typename Super::const_iterator;

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

    iterator erase(iterator it)
    {
        return Super::erase(it);
    }

    iterator erase(const_iterator it)
    {
        return Super::erase(it);
    }

    iterator erase(const_iterator first, const_iterator last)
    {
        return Super::erase(first, last);
    }

    template <typename Fn> requires std::is_invocable_r_v<bool, Fn, const_iterator>
    size_t erase_if(Fn&& fn)
    {
        size_t eraseCount = 0;

        auto it = Super::cbegin();
        while (it != Super::cend())
        {
            if (std::invoke(fn, it))
            {
                it = Super::erase(it);
                ++eraseCount;
            }
            else
            {
                ++it;
            }
        }

        return eraseCount;
    }

    template <typename...Args>
    decltype(auto) try_emplace(std::string_view sv, Args&&...args)
    {
        return Super::try_emplace(std::string{ sv }, std::forward<Args>(args)...);
    }
};


template <typename Value>
using Dictionary = DictionaryTemplate<std::map, Value, std::less<>>;

template <typename Value>
using UnorderedDictionary = 
    DictionaryTemplate<std::unordered_map, Value, DictionaryHash, DictionaryEq>;