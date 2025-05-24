#pragma once
#include "../TagsComponent.h"
#include "../../core/Result.h"

class TagCategory
{
public:
	static constexpr std::string_view kName = "name";
	static constexpr std::string_view kRelation = "relation";
	static constexpr std::string_view kChildRole = "childRole";
	static constexpr std::string_view kParentRole = "parentRole";
	static constexpr std::string_view kFilter = "filter";

private:
	TagCategory() = default;
};

class Tag
{ 
public:
	static constexpr char kSeparator = ':';

	Tag() = default;
	Tag(std::string_view input);
	Tag(std::string_view cat, std::string_view val) :
		category_(std::string{cat}), value_(std::string{val}) {}
	
	const std::string& GetCategory() const { return category_; }
	const std::string& GetValue() const { return value_; }

	bool IsValid() const { return !(category_.empty() || value_.empty()); }

	std::string Compose() const;

	static Result<Tag> Decompose(std::string_view input);

private:	
	std::string category_;
	std::string value_;
};


inline Tag MakeNameTag(std::string_view nm)
{
	return Tag{ TagCategory::kName, nm };
}
inline Tag MakeRelationTag(std::string_view val)
{
	return Tag{ TagCategory::kRelation, val };
}