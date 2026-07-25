#pragma once
#include <format>
#include <optional>
#include <string_view>

template <typename T>
struct std::formatter<std::optional<T>> : std::formatter<T> 
{
    auto format(const std::optional<T>& opt, format_context& ctx) const 
    {
        if (opt.has_value()) 
        {
            return std::formatter<T>::format(*opt, ctx);
        }

        return ctx.out();
    }
};