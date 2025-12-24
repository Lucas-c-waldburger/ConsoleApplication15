#pragma once
#include "System.h"
#include "../file/FilePathUtility.h"
#include "../core/commonObjects.h"

class TextureRepository;

class SerializationSystem : public System
{
public:
	static constexpr std::string_view kJsonEntitiesKey = "entities";
	static constexpr std::string_view kJsonEntityIdKey = "id";
	static constexpr std::string_view kJsonComponentsKey = "components";

	enum class ErrorProtocol
	{
		DefaultConstructInvalidComponents,
		OmitInvalidComponents,
		ErrorOutOnInvalidComponent
	};

	Result<Void> SerializeState(std::string_view jsonFilename, 
								TextureRepository& textureRepo);
	std::vector<Error> DeserializeState(std::string_view jsonFilename, 
										TextureRepository& textureRepo);
};