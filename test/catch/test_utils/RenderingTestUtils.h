#pragma once
#include "../../../core/Result.h"
#include "../../../core/commonObjects.h"

class Entity;
class TextureRepository;

namespace test {

Result<Void> QuickRender(const Entity& entity, const TextureRepository& textureRepo);








} // test