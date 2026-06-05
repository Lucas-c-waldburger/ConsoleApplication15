#pragma once
#include "System.h"
#include "../file/FilePathUtility.h"
#include "../core/commonObjects.h"

class B2World;
class TextureRepository;
class SDLInputSystem;
class AudioBank;
struct SDL_Renderer;

class SerializationSystem : public System
{
public:
	struct Filepaths
	{
		std::string texturesPath;
		std::string audioPath;
		std::string entitiesPath;
	};

	static Result<Void> 
	SerializeState(const Filepaths& filepaths, const TextureRepository& textureRepo,
				   const AudioBank& audioBank);

	static std::vector<Error> 
	DeserializeState(const Filepaths& filepaths, B2World& world, 
					 TextureRepository& textureRepo, SDLInputSystem& inputSystem,
					 AudioBank& audioBank, SDL_Renderer* renderer);
};