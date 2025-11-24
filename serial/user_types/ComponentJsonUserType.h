#pragma once
#include "../SerializationConcepts.h"
#include "RenderableJsonUserTypes.h"
#include "../../components/ComponentIncludes.h"

#define DEF_COMPONENT_NAME(cmpType)										\
template <> struct ComponentName<cmpType> {								\
	static constexpr std::string_view value = #cmpType;					\
}																	    

#define DEF_COMPONENT_SERIALIZABLE(cmpType, ...)						\
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(cmpType, __VA_ARGS__)				\
DEF_COMPONENT_NAME(cmpType)






DEF_COMPONENT_SERIALIZABLE(Transform, position, rotation, scale);
DEF_COMPONENT_SERIALIZABLE(CameraTarget, offset, followSpeed);
DEF_COMPONENT_SERIALIZABLE(TextRenderableComponent, writer, formatting, profile);
DEF_COMPONENT_SERIALIZABLE(SpriteRenderableComponent, profile);
