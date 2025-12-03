#pragma once
#include "../../deps/catch/catch_amalgamated.hpp"
#include "../../core/Result.h"

#define CAPTURE_RESULT(result_) do { \
	if (!result_.Success()) { CATCH_MAKE_MSG(result_.GetError().GetMessage()); } \
} while(0)

#define CHECK_RESULT(result_) do { \
	CAPTURE_RESULT(result_); \
	CHECK(result_.Success()); \
} while(0)

#define REQUIRE_RESULT(result_) do { \
	CAPTURE_RESULT(result_); \
	REQUIRE(result_.Success()); \
} while(0)