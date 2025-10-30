#pragma once
#include "../../../core/Result.h"
#include "../../../core/commonObjects.h"

class Entity;
class NewTextureRepository;

namespace test {

Result<Void> QuickRender(const Entity& entity, const NewTextureRepository& textureRepo);








} // test