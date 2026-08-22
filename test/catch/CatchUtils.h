#pragma once
//#include "../../core/Result.h"

//#ifdef WARN
//	#undef WARN
//#endif

#include "../../deps/catch/catch_amalgamated.hpp"

#ifdef GetMessage
	#undef GetMessage
#endif

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