#pragma once
#include "../core/SizedEnum.h"


template <typename T>
concept SomeInputSourceEnum = SomeSizedEnum<T> &&
							  std::is_signed_v<std::underlying_type_t<T>>;