#pragma once
#include "BaseComponent.h"
#include <string>
#include <unordered_set>

struct Tags : BaseComponent<Tags>
{
    std::unordered_set<std::string> tags;
};