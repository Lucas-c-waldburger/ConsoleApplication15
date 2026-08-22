#include "Logger.h"

Logger::~Logger() { EndSession(); }

void Logger::StartSession(const std::string& logFile)
{
    logger_ = std::make_shared<spdlog::logger>("logger");

    auto& sinks = logger_->sinks();
    sinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

    if (!logFile.empty())
    {
        sinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFile, true));
    }

    logger_->set_level(spdlog::level::trace);
    logger_->set_pattern("[%H:%M:%S] [%^%l%$] %v");
}

void Logger::EndSession()
{
    if (logger_)
    {
        logger_->flush();
        logger_.reset();
    }
}

spdlog::level::level_enum Logger::GetLogLevel()
{
    if (logger_)
    {
        return logger_->level();
    }

    return spdlog::level::level_enum::off;
}

void Logger::SetLogLevel(spdlog::level::level_enum lvl)
{
    if (logger_)
    {
        logger_->set_level(lvl);
    }
}


std::shared_ptr<spdlog::logger>& Logger::Get() { return logger_; }
