#pragma once
#include "System.h"

class NewTextureRepository;

class RenderablePreProcessor : public System
{
public:
	void Update(const NewTextureRepository& textureRepo);

	size_t GetRenderCallCount() const noexcept { return renderCallCount_; }
	size_t GetDebugDrawPointCount() const noexcept { return debugDrawPointCount_; }

private:
	void ClearGlyphCache(Entity& entity);

	size_t renderCallCount_ = 0;
	size_t debugDrawPointCount_ = 0;
};