#pragma once
#include "AudioEnums.h"
#include <string>

struct AudioDescriptor
{
    AudioType audioType = AudioType::Unknown;
    std::string name;
    std::string filepath;
};
