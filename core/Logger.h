#pragma once
#include <iostream>
#include <fstream>
#include <iomanip>
#include <format>
#include <memory>
#include <vector>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
//#include "../deps/spdlog/include/spdlog/spdlog.h"
//#include "../deps/spdlog/include/spdlog/sinks/stdout_color_sinks.h"
//#include "../deps/spdlog/include/spdlog/sinks/basic_file_sink.h"

#ifdef GetMessage
#undef GetMessage
#endif

class Logger
{
public:
    ~Logger();

    static void StartSession(const std::string& logFile = {});
    static void EndSession();

    template <typename T, typename...Args>
        requires (std::derived_from<T, spdlog::sinks::sink> &&
                  std::constructible_from<T, Args...>)
    static std::shared_ptr<T> AddSink(Args&&...args);

    static spdlog::level::level_enum GetLogLevel();

    static void SetLogLevel(spdlog::level::level_enum lvl);

    static std::shared_ptr<spdlog::logger>& Get();

private:
    Logger() = default;

    static inline std::shared_ptr<spdlog::logger> logger_;
};

template <typename T, typename...Args>
    requires (std::derived_from<T, spdlog::sinks::sink>&&
              std::constructible_from<T, Args...>)
inline std::shared_ptr<T> Logger::AddSink(Args&&...args)
{
    if (!logger_)
    {
        return nullptr;
    }

    auto sink = std::make_shared<T>(std::forward<Args>(args)...);

    auto& sinks = logger_->sinks();
    sinks.emplace_back(sink);

    return sink;
}

#define LOG_DEBUG(...) \
    do { if (Logger::Get()) { Logger::Get()->debug(__VA_ARGS__); } } while(0)
#define LOG_INFO(...) \
    do { if (Logger::Get()) { Logger::Get()->info(__VA_ARGS__); } } while(0)
#define LOG_WARNING(...) \
    do { if (Logger::Get()) { Logger::Get()->warn(__VA_ARGS__); } } while(0)
#define LOG_ERROR(...) \
    do { if (Logger::Get()) { Logger::Get()->error(__VA_ARGS__); } } while(0)
#define LOG_CRITICAL(...) \
    do { if (Logger::Get()) { Logger::Get()->critical(__VA_ARGS__); } } while(0)

#define LOG_DEBUG_FMT(fmtStr, ...)  \
    do { if (Logger::Get()) { Logger::Get()->debug(fmtStr, __VA_ARGS__); } } while(0)
#define LOG_INFO_FMT(fmtStr, ...) \
    do { if (Logger::Get()) { Logger::Get()->info(fmtStr, __VA_ARGS__); } } while(0)
#define LOG_WARNING_FMT(fmtStr, ...) \
    do { if (Logger::Get()) { Logger::Get()->warn(fmtStr, __VA_ARGS__); } } while(0)
#define LOG_ERROR_FMT(fmtStr, ...) \
    do { if (Logger::Get()) { Logger::Get()->error(fmtStr, __VA_ARGS__); } } while(0)
#define LOG_CRITICAL_FMT(fmtStr, ...) \
    do { if (Logger::Get()) { Logger::Get()->critical(fmtStr, __VA_ARGS__); } } while(0)

//#define LOG_DEBUG(...) Logger::Log(LogLevel::DEBUG, __VA_ARGS__)
//#define LOG_INFO(...) Logger::Log(LogLevel::INFO, __VA_ARGS__)
//#define LOG_WARNING(...) Logger::Log(LogLevel::WARNING, __VA_ARGS__)
//#define LOG_ERROR(...) Logger::Log(LogLevel::ERROR, __VA_ARGS__)
//#define LOG_CRITICAL(...) Logger::Log(LogLevel::CRITICAL, __VA_ARGS__)
//
//#define LOG_DEBUG_FMT(fmtStr, ...) Logger::Log(LogLevel::DEBUG, std::format(fmtStr, __VA_ARGS__))
//#define LOG_INFO_FMT(fmtStr, ...) Logger::Log(LogLevel::INFO, std::format(fmtStr, __VA_ARGS__))
//#define LOG_WARNING_FMT(fmtStr, ...) Logger::Log(LogLevel::WARNING, std::format(fmtStr, __VA_ARGS__))
//#define LOG_ERROR_FMT(fmtStr, ...) Logger::Log(LogLevel::ERROR, std::format(fmtStr, __VA_ARGS__))
//#define LOG_CRITICAL_FMT(fmtStr, ...) Logger::Log(LogLevel::CRITICAL, std::format(fmtStr, __VA_ARGS__))