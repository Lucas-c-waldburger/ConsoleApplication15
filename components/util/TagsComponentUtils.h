#pragma once
#include "../../core/Result.h"

class TagCategory
{
public:
	static constexpr std::string_view kName = "name";
	static constexpr std::string_view kChildName = "childName";
	static constexpr std::string_view kRelation = "relation";
	static constexpr std::string_view kChildRole = "childRole";
	static constexpr std::string_view kParentRole = "parentRole";
	static constexpr std::string_view kFilter = "filter";

private:
	TagCategory() = default;
};

struct Tag
{
public:
	static constexpr char kSeparator = ':';

	std::string_view category;
	std::string_view value;

	static std::string Compose(std::string_view category, std::string_view value);
	static std::string Compose(const Tag& tag);
	static Tag Decompose(std::string_view input);
};