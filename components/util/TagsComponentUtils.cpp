#include "TagsComponentUtils.h"

std::string Tag::Compose(std::string_view category, std::string_view value)
{
    std::string composed{};
    composed.reserve(category.size() + value.size() +
        (!category.empty()) ? 1 : 0);

    if (!category.empty())
    {
        composed.append(category);
        composed.push_back(kSeparator);
    }

    composed.append(value);

    return composed;
}

std::string Tag::Compose(const Tag& tag)
{
    return Compose(tag.category, tag.value);
}

Tag Tag::Decompose(std::string_view input) 
{
    Tag decomposed{};
    size_t pos = input.find(':');

    if (pos != std::string_view::npos) 
    {
        decomposed.category = input.substr(0, pos);
        decomposed.value = input.substr(pos + 1);
    }
    else 
    {
        decomposed.value = input;
    }

    return decomposed;
}
