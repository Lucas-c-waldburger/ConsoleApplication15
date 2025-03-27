#pragma once
#include <functional>
#include <vector>
#include <filesystem>

enum class ReturnSignal 
{ 
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