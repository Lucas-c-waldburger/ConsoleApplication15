#pragma once
#include "System.h"
#include "../file/FilePathUtility.h"
#include "../core/commonObjects.h"


class SerializationSystem : public System
{
public:
	static constexpr std::string_view kJsonEntitiesKey = "entities";
	static constexpr std::string_view kJsonComponentsKey = "components";

	enum class ErrorProtocol
	{
		DefaultConstructInvalidComponents,
		OmitInvalidComponents,
		ErrorOutOnInvalidComponent
	};

	Result<Void> SerializeEntities(const ResourcePathResult& path);
	std::vector<Error> DeserializeEntities(const ResourcePathResult& path);
};