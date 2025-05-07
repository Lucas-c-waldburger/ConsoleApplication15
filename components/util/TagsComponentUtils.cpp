#include "TagsComponentUtils.h"


std::string Tag::Compose() const
{
	if (!IsValid())
	{
		return {};
	}

	return category_ + std::string{kSeparator} + value_;
}

Result<Tag> Tag::Decompose(std::string_view input)
{
	size_t pos = input.find(kSeparator);
	if (pos == std::string_view::npos)
	{
		return MAKE_ERROR_FMT("Tag input ill-formed ('{}')", input);
	}

	return Tag{ input.substr(0, pos), input.substr(pos + kSeparator.size()) };
}