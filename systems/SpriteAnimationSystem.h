#pragma once
#include "System.h"
#include "Pausable.h"

class TextureRepository;

class SpriteAnimationSystem : public System,
							  public Pausable<SpriteAnimationSystem>
{
public:
	friend class Pausable<SpriteAnimationSystem>;

	void Update(const TextureRepository& textureRepo);
};
