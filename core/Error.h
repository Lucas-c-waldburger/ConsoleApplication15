#pragma once
#include <iostream>
#include <source_location>

enum class Color
{
    Reset = 0,
    Black = 30,
    Red = 31,
    Green = 32,
    Yellow = 33,
    Blue = 34,
    Magenta = 35,
    Cyan = 36,
    White = 37
};

template <Color>
struct TerminalColor { const char* text; };

template <Color clr>
static std::ostream& operator<<(std::ostream& os, TerminalColor<clr>&& tc)
{
    os << "\033[" << static_cast<int>(clr) << "m"
        << tc.text
        << "\033[" << static_cast<int>(Color::Reset) << "m";
    return os;
}

enum class Severity { Warn = 1, NonCritical, Critical };

class Error
{
public:
    Error() = default;
    Error(std::source_location srcLoc, const char* msg, Severity sev = Severity::Critical) :
        srcLocation_(std::move(srcLoc)), message_(msg), severity_(sev) {}

    const char* GetMessage() const { return message_; }
    Severity GetSeverity() const { return severity_; }

    friend std::ostream& operator<<(std::ostream& os, const Error& err);

private:
    static constexpr const char* kWarn = "WARNING";
    static constexpr const char* kNonCritical = "NON-CRITICAL";
    static constexpr const char* kCritical = "CRITICAL";
    static constexpr const char* GetSeverityString(const Severity sev)
    {
        switch (sev)
        {
        case Severity::Warn: return kWarn;
        case Severity::NonCritical: return kNonCritical;
        case Severity::Critical: return kCritical;
        }
    }

    std::source_location srcLocation_;
    const char* message_;
    Severity severity_ = Severity::Warn;
};

static std::ostream& operator<<(std::ostream& os, const std::source_location& l)
{
    os << "  File     : " << l.file_name() << '\n'
       << "  Function : " << l.function_name() << '\n'
       << "  Line     : " << l.line() << '\n'
       << "  Column   : " << l.column();

    return os;
}

static std::ostream& operator<<(std::ostream& os, const Error& err)
{
    os << TerminalColor<Color::Red>("!!! ERROR !!!") << '\n'
       << TerminalColor<Color::Yellow>("Severity : ") << Error::GetSeverityString(err.severity_) << '\n'
       << TerminalColor<Color::Yellow>("Location :\n") << err.srcLocation_ << '\n'
       << TerminalColor<Color::Yellow>("Message  : ") << '"' << err.message_ << "\"\n";

    return os;
}

#define MAKE_ERROR(msg, ...) Error{std::source_location::current(), msg, __VA_ARGS__}