#pragma once
#include "../core/Result.h"
#include "../core/commonObjects.h"

class B2World;

Result<Void> BufferCollisionEvents(const B2World* world);

