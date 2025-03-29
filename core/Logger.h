#pragma once
#include <iostream>
#include <fstream>
#include <iomanip>
#include <format>
#include "TerminalUtils.h"

class LogLevel
{
public:
    enum Level
    {
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        CRITICAL
    };

    friend std::ostream& operator<<(std::ostream& os, LogLevel::Level lvl);

private:
    static constexpr const char* kDebugStr = "DEBUG";
    static constexpr const char* kInfoStr = "INFO";
    static constexpr const char* kWarnStr = "WARNING";
    static constexpr const char* kErrorStr = "ERROR";
    static constexpr const char* kCriticalStr = "CRITICAL";
    static constexpr const char* kUnknownStr = "UKNOWN";
};

static std::ostream& operator<<(std::ostream& os, LogLevel::Level lvl);

class Logger
{
public:
    ~Logger()
    {
        Log(LogLevel::INFO, "Program Ended");
        CloseFilestream();
    }

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    static void StartSession(const std::string& logFile = {}, bool enableConsole = true)
    {
        return Logger::Get().StartSessionImpl(logFile, enableConsole);
    }

    static void EndSession()
    {
        return Logger::Get().EndSessionImpl();
    }

    static void SetLogFile(const std::string& fileName)
    {
        return Logger::Get().SetLogFile(fileName);
    }

    static bool IsRunning()
    {
        return Logger::Get().flags_ & Logger::Flag::SessionStarted;
    }

    template <typename...Ts>
    static void Log(LogLevel::Level lvl, Ts&&...data)
    {
        return Logger::Get().LogImpl(lvl, std::forward<Ts>(data)...);
    }

    static Logger& Get();

private:
    enum Flag : uint8_t
    {
        SessionStarted = 1 << 0,
        WriteToConsole = 1 << 1
    };

    Logger() = default;

    template <typename...Ts>
    void LogImpl(LogLevel::Level lvl, Ts&&...data)
    {
        if ((flags_ & Flag::SessionStarted) == 0)
        {
            return;
        }

        if (flags_ & Flag::WriteToConsole)
        {
            LogHeader(std::cout, lvl);
            ((std::cout << std::forward<Ts>(data)), ...) << '\n';
        }
        if (fileStream_.is_open())
        {
            LogHeader(fileStream_, lvl);
            ((fileStream_ << std::forward<Ts>(data)), ...) << '\n';
        }
    }

    void StartSessionImpl(const std::string& logFile, bool enableConsole);
    void EndSessionImpl();
    void SetLogFileImpl(const std::string& fileName);
    void CloseFilestream();

    static void LogTime(std::ostream& os);
    static void LogHeader(std::ostream& os, LogLevel::Level lvl);

    std::ofstream fileStream_;
    uint8_t flags_ = 0;
};

#define LOG_DEBUG(...) Logger::Log(LogLevel::DEBUG, __VA_ARGS__)
#define LOG_INFO(...) Logger::Log(LogLevel::INFO, __VA_ARGS__)
#define LOG_WARNING(...) Logger::Log(LogLevel::WARNING, __VA_ARGS__)
#define LOG_ERROR(...) Logger::Log(LogLevel::ERROR, __VA_ARGS__)
#define LOG_CRITICAL(...) Logger::Log(LogLevel::CRITICAL, __VA_ARGS__)

#define LOG_DEBUG_FMT(fmtStr, ...) Logger::Log(LogLevel::DEBUG, std::format(fmtStr, __VA_ARGS__))
#define LOG_INFO_FMT(fmtStr, ...) Logger::Log(LogLevel::INFO, std::format(fmtStr, __VA_ARGS__))
#define LOG_WARNING_FMT(fmtStr, ...) Logger::Log(LogLevel::WARNING, std::format(fmtStr, __VA_ARGS__))
#define LOG_ERROR_FMT(fmtStr, ...) Logger::Log(LogLevel::ERROR, std::format(fmtStr, __VA_ARGS__))
#define LOG_CRITICAL_FMT(fmtStr, ...) Logger::Log(LogLevel::CRITICAL, std::format(fmtStr, __VA_ARGS__))