#pragma once
#include "../Serialization.h"
#include "../../components/ComponentIncludes.h"


DEF_COMPONENT_SERIALIZABLE(Transform, position, scale, rotation);
DEF_COMPONENT_SERIALIZABLE(CameraTarget, offset, followSpeed);
DEF_COMPONENT_SERIALIZABLE(TextRenderableComponent, writer, formatting, profile);
DEF_COMPONENT_SERIALIZABLE(SpriteRenderableComponent, profile);


