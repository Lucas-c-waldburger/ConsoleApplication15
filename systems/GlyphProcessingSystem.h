#pragma once
#include "System.h"

class Entity;
class GlyphAtlas;
struct Transform;

class GlyphFormattingSystem : public System
{
public:
	void Update(const GlyphAtlas& glyphAtlas);
	void EntityDestroyed(Entity& entity);
};