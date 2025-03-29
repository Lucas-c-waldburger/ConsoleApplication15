#pragma once
#include "Error.h"
#include "Logger.h"

template <typename T>
class Result
{
public:
    using ResultType = T;

    Result(T val) : status_(hasValue) { new (&value_) T(std::move(val)); }
    Result(Error err) : status_(0) { new (&error_) Error(std::move(err)); }
    ~Result()
    {
        if (status_ & hasValue) { value_.~T(); }

        else { error_.~Error(); }
    }

    Result(Result&& other) noexcept : status_(other.status_)
    {
        if (status_ & hasValue) { new (&value_) T(std::move(other.value_)); }

        else { new (&error_) Error(std::move(other.error_)); }
    }

    Result& operator=(Result&& other) noexcept
    {
        if (this == &other) { return *this; }

        this->~Result();

        status_ = other.status_;
        if (status_ & hasValue) { new (&value_) T(std::move(other.value_)); }

        else { new (&error_) Error(std::move(other.error_)); }

        return *this;
    }

    Result() = delete;
    Result(const Result&) = delete;
    Result& operator=(const Result&) = delete;

    bool Success() const
    {
        status_ |= checked;
        return status_ & hasValue;
    }

    T& GetValue() { return GetValueInternal(); }
    const T& GetValue() const { return GetValueInternal(); }
    Error& GetError() { return GetErrorInternal(); }
    const Error& GetError() const { return GetErrorInternal(); }

    T& operator*() { return GetValueInternal(); }
    const T& operator*() const { return GetValueInternal(); }
    T* operator->() { return &GetValueInternal(); }
    const T* operator->() const { return &GetValueInternal(); }

private:
    T& GetValueInternal()
    {
        if ((status_ & checked) == 0)
        {
            LOG_WARNING("Result has been retrieved without checking Success().");
        }
        if ((status_ & hasValue) == 0)
        {
            LOG_CRITICAL("The result contained an error! - Exiting program...");
            std::exit(EXIT_FAILURE);
        }

        return value_;
    }

    Error& GetErrorInternal()
    {
        if ((status_ & checked) == 0)
        {
            LOG_WARNING("Error has been retrieved without checking Success().");
        }
        if (status_ & hasValue)
        {
            LOG_CRITICAL("The result contained a value! - Exiting program...");
            std::exit(EXIT_FAILURE);
        }

        return error_;
    }

    union
    {
        T value_;
        Error error_;
    };

    enum { hasValue = 1 << 0, checked = 1 << 1 };
    mutable uint8_t status_;
};

#define ASSERT_RESULT(result) do { \
    if constexpr (std::is_lvalue_reference_v<decltype((result))>) { \
        if (!(result).Success()) { \
            Logger::Get().Log(LogLevel::ERROR, (result).GetError()); \
            std::exit(EXIT_FAILURE); \
        } \
    } else { \
        auto asrt_res_tmp__ = (result); \
        if (!asrt_res_tmp__.Success()) { \
            Logger::Get().Log(LogLevel::ERROR, (result).GetError()); \
            std::exit(EXIT_FAILURE); \
        } \
    } \
} while(0)

#define LOG_IF_ERROR(result) do { \
    if (!(result).Success()) { \
        Logger::Get().Log(LogLevel::ERROR, (result).GetError()); \
    } \
} while(0)


// TRY MACRO //
#define zz_CONCAT(a, b) a##b
#define z_CONCAT(a, b) zz_CONCAT(a, b)
#define CONCAT(a, b) z_CONCAT(a, b)

#define RETURN_ARG_COUNT(_1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, count, ...) count
#define EXPAND_ARGS(args) RETURN_ARG_COUNT args
#define COUNT_ARGS_MAX8(...) EXPAND_ARGS((__VA_ARGS__, 8, 7, 6, 5, 4, 3, 2, 1 , 0))
#define GLUE_MACRO(x, y) x y
#define CALL_OVERLOAD(name, ...) GLUE_MACRO(CONCAT(name, COUNT_ARGS_MAX8(__VA_ARGS__)), (__VA_ARGS__))

#define RETURN_IF_FAILURE(result_) do { \
    if(!(result_).Success()) { return (result_).GetError(); } \
} while(0)

#define TRY_ASSIGN_OR_RETURN_INTERNAL(dest, src, temp, returnValue) \
    auto temp = (src); \
    if (!temp.Success()) { return (returnValue); } \
    dest = std::move(temp).GetValue();

#define TRY_ASSIGN(dest, src) TRY_ASSIGN_INTERNAL(dest, src, CONCAT(_result, __COUNTER__))
#define TRY_ASSIGN_INTERNAL(dest, src, temp) TRY_ASSIGN_OR_RETURN_INTERNAL(dest, src, temp, temp.GetError())

#define TRY_INVOKE_1(expr) do { auto ltil_tmp__ = (expr); RETURN_IF_FAILURE(ltil_tmp__); } while(0)
#define TRY_INVOKE_2(src, dest) TRY_ASSIGN(auto dest, src)

#define TRY(...) CALL_OVERLOAD(TRY_INVOKE_, __VA_ARGS__)