#pragma once
#include <iostream>
#include <source_location>
#include "TerminalUtils.h"
#include <format>

class Error
{
public:
    Error() = default;
    Error(std::source_location srcLoc, std::string msg) :
        srcLocation_(std::move(srcLoc)), message_(std::move(msg)) {}

    const std::string& GetMessage() const { return message_; }

    friend std::ostream& operator<<(std::ostream& os, const Error& err);

private:
    std::source_location srcLocation_;
    std::string message_;
};

static std::ostream& operator<<(std::ostream& os, const std::source_location& l);

#define MAKE_ERROR(msg) Error{std::source_location::current(), msg}
#define MAKE_ERROR_FMT(msg, ...) Error{std::source_location::current(), std::format(msg, __VA_ARGS__)}