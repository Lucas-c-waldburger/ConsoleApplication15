#include "Monitoring.h"

void SimpleTrigger::AddObserver(Callback&& observer)
{
    observers_.push_back(std::move(observer));
}

void SimpleTrigger::Trigger()
{
    for (int i = 0; i < static_cast<int>(observers_.size()); i++)
    {
        if (!observers_[i] || observers_[i]() == ReturnSignal::StopObserving)
        {
            int backIdx = observers_.size() - 1;
            if (backIdx != i)
            {
                std::swap(observers_[i], observers_[backIdx]);
            }

            observers_.pop_back();

            --i;
        }
    }
}

void FileMonitor::Check(const double deltaTime)
{
    if (!running_)
    {
        return;
    }
    if ((counter_ += static_cast<int>(deltaTime)) < frequencyMs_)
    {
        return;
    }

    auto currentWriteTime = std::filesystem::last_write_time(filePath_);
    if (currentWriteTime != lastWriteTime_)
    {
        Trigger();
        lastWriteTime_ = currentWriteTime;
    }

    counter_ = 0;
}