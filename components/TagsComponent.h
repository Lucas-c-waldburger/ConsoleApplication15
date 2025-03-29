#pragma once
#include "BaseComponent.h"
#include <string>
#include <unordered_set>

struct Tags : BaseComponent<Tags, 6>
{
    std::unordered_set<std::string> tags;
};