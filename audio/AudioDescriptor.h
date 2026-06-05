#pragma once
#include "AudioEnums.h"
#include <string>

struct AudioDescriptor
{
    AudioType audioType = AudioType::Sound;
    std::string name;
    std::string filepath;
};
