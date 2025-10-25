#define CATCH_CONFIG_RUNNER
#include "../../deps/catch/catch_amalgamated.hpp"
#include "../../core/Result.h"

#define CAPTURE_RESULT(result_) do { \
	if (!result_.Success()) { CAPTURE(result_.GetError().GetMessage()); } \
} while(0)

#define CHECK_RESULT(result_) do { \
	CAPTURE_RESULT(result_); \
	CHECK(result_.Success()); \
} while(0)

#define REQUIRE_RESULT(result_) do { \
	CAPTURE_RESULT(result_); \
	REQUIRE(result_.Success()); \
} while(0)

//#define CATCH_RETURN_IF_FAILURE(result_) do { \
//	if (!result.Success()) CAPTURE(result_.GetError()); REQUIRE(0); \
//} while(0)
//
//#define CATCH_TRY_ASSIGN_OR_RETURN_INTERNAL(dest, src, temp, returnValue) \
//    auto temp = (src); \
//    if (!temp.Success()) { return (returnValue); } \
//    dest = std::move(temp).GetValue();
//
//#define TRY_ASSIGN(dest, src) TRY_ASSIGN_INTERNAL(dest, src, CONCAT(_result, __COUNTER__))
//#define TRY_ASSIGN_INTERNAL(dest, src, temp) TRY_ASSIGN_OR_RETURN_INTERNAL(dest, src, temp, temp.GetError())
//
//#define CATCH_TRY_INVOKE_1(expr) do { \
//	auto cltil_tmp__ = (expr); \
//	RETURN_IF_FAILURE(ltil_tmp__); } while(0)
//#define CATCH_TRY_INVOKE_2(src, dest) TRY_ASSIGN(auto dest, src)
//
//#define CATCH_REQUIRE_TRY(...) CALL_OVERLOAD(CATCH_TRY_INVOKE_, __VA_ARGS__)