#pragma once
#include "ComponentConcepts.h"
#include "util/TagsComponentUtils.h"
#include <string>
#include <unordered_set>

struct Tags
{
    std::unordered_set<std::string> tags;

    bool operator==(const Tags&) const = default;
};