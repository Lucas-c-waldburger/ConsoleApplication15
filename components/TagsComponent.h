#pragma once
#include "BaseComponent.h"
#include "util/TagsComponentUtils.h"
#include <string>
#include <unordered_set>

struct Tags : BaseComponent<Tags>
{
    std::unordered_set<std::string> tags;
};