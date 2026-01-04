#include "Logger.h"

std::ostream& operator<<(std::ostream& os, LogLevel::Level lvl)
{
    if (os.rdbuf() == std::cout.rdbuf() || os.rdbuf() == std::cerr.rdbuf())
    {
        switch (lvl)
        {
        case LogLevel::DEBUG: os << TerminalColor<Color::Green>(LogLevel::kDebugStr); break;
        case LogLevel::INFO: os << TerminalColor<Color::Cyan>(LogLevel::kInfoStr); break;
        case LogLevel::WARNING: os << TerminalColor<Color::Yellow>(LogLevel::kWarnStr); break;
        case LogLevel::ERROR: os << TerminalColor<Color::Red>(LogLevel::kErrorStr); break;
        case LogLevel::CRITICAL: os << TerminalColor<Color::BrightRed>(LogLevel::kCriticalStr); break;
        default: os << TerminalColor<Color::Pink>(LogLevel::kUnknownStr); break;
        }
    }
    else
    {
        switch (lvl)
        {
        case LogLevel::DEBUG: os << LogLevel::kDebugStr; break;
        case LogLevel::INFO: os << LogLevel::kInfoStr; break;
        case LogLevel::WARNING: os << LogLevel::kWarnStr; break;
        case LogLevel::ERROR: os << LogLevel::kErrorStr; break;
        case LogLevel::CRITICAL: os << LogLevel::kCriticalStr; break;
        default: os << LogLevel::kUnknownStr; break;
        }
    }
    return os;
}

Logger& Logger::Get()
{
    static std::unique_ptr<Logger> instance;
    if (!instance)
    {
        instance = std::unique_ptr<Logger>(new Logger());
    }

    return *instance;
}

void Logger::StartSessionImpl(const std::string& logFile, bool enableConsole)
{
    if (flags_ & Flag::SessionStarted) { return; }

    if (!logFile.empty())
    {
        fileStream_.open(logFile, std::ios::app);
        if (!fileStream_.is_open())
        {
            std::cerr << "Error: Unable to open log file: " << logFile << '\n';
        }
    }

    flags_ |= (enableConsole) ? Flag::WriteToConsole : 0;
    flags_ |= Flag::SessionStarted;
}

void Logger::EndSessionImpl()
{
    if (flags_ & Flag::SessionStarted)
    {
        CloseFilestream();
    }
    flags_ = 0;
}

void Logger::SetLogFileImpl(const std::string& fileName)
{
    if ((flags_ & Flag::SessionStarted) == 0) { return; }

    CloseFilestream();

    fileStream_.open(fileName, std::ios::app);
    if (!fileStream_.is_open())
    {
        std::cerr << "Error: Unable to open log file: " << fileName << '\n';
    }
}

void Logger::CloseFilestream()
{
    if (fileStream_.is_open())
    {
        fileStream_ << std::endl;
        fileStream_.flush();
        fileStream_.close();
    }
}

void Logger::LogTime(std::ostream& os)
{
    time_t t = time(nullptr);
    struct tm tm_info;

    if (localtime_s(&tm_info, &t) == 0)
    {
        os << std::put_time(&tm_info, "%Y-%m-%d %H:%M:%S");
    }
    else
    {
        os << "Error converting elapsed";
    }
}

void Logger::LogHeader(std::ostream& os, LogLevel::Level lvl)
{
    os << "["; 
    LogTime(os); 
    os << "] [" << lvl << "] : ";
}

std::ostream& Logger::GetOStream()
{
    return std::cout;
}