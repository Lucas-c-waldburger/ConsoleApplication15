#pragma once

enum class AudioType
{
    Unknown,
    Sound,
    Music
};

enum class AudioPlayCommand
{
    None,
    Pause,
    Resume,
    Restart,
    Stop
};

enum class AudioStatus
{
    Playing,
    Paused,
    Stopping,
    Stopped,
    Staged
};
