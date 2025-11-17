#pragma once
#include "../../deps/nlohmann/json.hpp"
#include "../../components/TransformComponent.h"


NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Transform, position, scale, rotation)

