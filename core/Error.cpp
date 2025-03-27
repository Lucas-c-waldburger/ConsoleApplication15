#include "Error.h"


std::ostream& operator<<(std::ostream& os, const std::source_location& l)
{
    os << "  File     : " << l.file_name() << '\n'
        << "  Function : " << l.function_name() << '\n'
        << "  Line     : " << l.line() << '\n'
        << "  Column   : " << l.column();

    return os;
}

std::ostream& operator<<(std::ostream& os, const Error& err)
{
    if (os.rdbuf() == std::cout.rdbuf() || os.rdbuf() == std::cerr.rdbuf())
    {
        os << "!!! ERROR !!!"
            << "\nLocation :" << err.srcLocation_
            << "\nMessage  : \"" << err.message_ << "\"\n";
    }
    else
    {
        os << TerminalColor<Color::Red>("!!! ERROR !!!") << '\n'
            << TerminalColor<Color::Yellow>("Location :\n") << err.srcLocation_ << '\n'
            << TerminalColor<Color::Yellow>("Message  : ") << '"' << err.message_ << "\"\n";
    }
    return os;
}