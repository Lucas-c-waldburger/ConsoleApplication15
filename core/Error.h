#pragma once
#include <iostream>
#include <source_location>
#include "TerminalUtils.h"
#include <format>
#include <spdlog/fmt/fmt.h>

class Error
{
public:
    Error() = default;
    Error(std::source_location srcLoc, std::string msg) :
        srcLocation_(std::move(srcLoc)), message_(std::move(msg)) {}

    const std::string& GetMessage() const { return message_; }
    const std::source_location& GetSourceLocation() const { return srcLocation_; }

    friend std::ostream& operator<<(std::ostream& os, const Error& err);

private:
    std::source_location srcLocation_;
    std::string message_;
};

#pragma warning(push)
#pragma warning(disable : 4244)

template <>
struct fmt::formatter<Error>
{
    constexpr auto parse(fmt::format_parse_context& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const Error& err, FormatContext& ctx) const
    {
        const auto& loc = err.GetSourceLocation();

        return fmt::format_to(
            ctx.out(),
            "{} ({}:{})",
            err.GetMessage(),
            loc.file_name(),
            loc.line()
        );
    }
};

static std::ostream& operator<<(std::ostream& os, const std::source_location& l);

#define MAKE_ERROR(msg) Error{std::source_location::current(), msg}
#define MAKE_ERROR_FMT(msg, ...) Error{std::source_location::current(), std::format(msg, __VA_ARGS__)}

#pragma warning(pop)