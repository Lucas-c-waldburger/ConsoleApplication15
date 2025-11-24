#pragma once
#include <system_error>

//template <typename T> requires std::is_enum_v<T> 
//class ErrorCategory;
//
//#define DEF_ERROR_CODES(errCodeEnumType) \
//template <> class ErrorCategory<errCodeEnumType> : public std::error_category { \
//public: \
//    static const ErrorCategory& instance() { \
//        static ErrorCategory inst; \
//        return inst; \
//    } \
//const char* name() const noexcept override { \
//    return #errCodeEnumType; \
//} \
//std::string message(int ev) const override; \
//}; \
//namespace std { \
//template <> struct is_error_code_enum<errCodeEnumType> : true_type {}; \
//} \
//inline std::error_code make_error_code(errCodeEnumType e) { \
//    return { static_cast<int>(e), ErrorCategory<errCodeEnumType>::instance() }; \
//} \
//std::string ErrorCategory<errCodeEnumType>::message(int ev) const
//
//
//enum class MyError
//{
//    RandomError
//};
//
//DEF_ERROR_CODES(MyError)
//{
//    switch (static_cast<MyError>(ev))
//    {
//
//    }
//}