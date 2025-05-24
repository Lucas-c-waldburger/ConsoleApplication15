#pragma once
#include <functional>
#include <vector>
#include <filesystem>
#include "Logger.h"

enum class ReturnSignal 
{ 
    Unknown,
    KeepObserving, 
    StopObserving, 
    Pause 
};

class SimpleTrigger
{
public:
    using Callback = std::function<ReturnSignal()>;

    void AddObserver(Callback&& observer);
    void Trigger();

private:
    std::vector<Callback> observers_;
};


class FileMonitor : public SimpleTrigger
{
public:
    FileMonitor() = default;
    explicit FileMonitor(std::string filePath, int frequency = 1000) : filePath_(std::move(filePath)),
        lastWriteTime_(std::filesystem::last_write_time(filePath_)), frequencyMs_(frequency) {}

    void Start() { running_ = true; counter_ = 0; }
    void Stop() { running_ = false; counter_ = 0; }
    bool IsRunning() const { return running_; }
    void Check(const double deltaTime);
    std::filesystem::file_time_type GetLastWriteTime() const { return lastWriteTime_; }

private:
    std::string filePath_;
    std::filesystem::file_time_type lastWriteTime_;
    bool running_ = false;
    int frequencyMs_ = 1000;
    int counter_ = 0;
};


class FileChangeMonitor
{
public:
    FileChangeMonitor() : lastCheckTime_(std::chrono::system_clock::now()), checkInterval_(1000) {}
    FileChangeMonitor(std::string filePath, long long intervalMs = 1000) :
        filePath_(std::move(filePath)), checkInterval_(intervalMs),
        lastCheckTime_(std::chrono::system_clock::now()) {}

    bool FileDidChange() const
    {
        auto now = std::chrono::system_clock::now();
        if (now - lastCheckTime_ < checkInterval_) 
        {
            return false; 
        }
        lastCheckTime_ = now;

        auto currentWriteTime = GetLastWriteTime();
        if (currentWriteTime != lastWriteTime_) 
        {
            lastWriteTime_ = currentWriteTime;
            return true;
        }
        return false;
    }

    void SetFilePath(std::string fp) { filePath_ = std::move(fp); }

    const std::string& GetFilePath() const { return filePath_; }

private:
    std::filesystem::file_time_type GetLastWriteTime() const 
    {
        try 
        {
            return std::filesystem::last_write_time(filePath_);
        }
        catch (const std::filesystem::filesystem_error& ex) 
        {
            LOG_ERROR("Filesystem exception caught: ", ex.what());
            return {};
        }
    }

    std::string filePath_;
    std::chrono::milliseconds checkInterval_;
    mutable std::chrono::system_clock::time_point lastCheckTime_;
    mutable std::filesystem::file_time_type lastWriteTime_;
};