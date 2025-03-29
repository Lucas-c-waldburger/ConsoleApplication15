#pragma once
#include <iostream>

using Entity_t = uint32_t;

static constexpr Entity_t kMaxEntities = 500;
static constexpr Entity_t kInvalidEntity = -1;
static constexpr size_t kInvalidIndex = kMaxEntities + 1;