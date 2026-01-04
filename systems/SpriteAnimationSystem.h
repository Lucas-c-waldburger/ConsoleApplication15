#pragma once
#include "System.h"

class TextureRepository;

class SpriteAnimationSystem : public System
{
public:
	void Update();
	void Update(const TextureRepository& textureRepo);
};
