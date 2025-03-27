#pragma once
#include <iostream>

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
    White = 37,
    Brown = 94,
    BrightBlack = 90,
    BrightRed = 91,
    BrightGreen = 92,
    BrightYellow = 93,
    BrightBlue = 94,
    BrightMagenta = 95,
    BrightCyan = 96,
    BrightWhite = 97,
    Pink = 200
};

template <Color>
struct TerminalColor 
{ 
    const char* text; 
};

template <Color clr>
static std::ostream& operator<<(std::ostream& os, TerminalColor<clr>&& tc)
{
    os << "\033[" << static_cast<int>(clr) << "m"
        << tc.text
        << "\033[" << static_cast<int>(Color::Reset) << "m";
    return os;
}