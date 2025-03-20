#pragma once
#include "Error.h"


struct Void {};

static constexpr const char* kResultCheckWarningMsg =
    "WARNING: Result has been retrieved without checking Success().";

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
            std::cerr << kResultCheckWarningMsg << ((status_ & hasValue) ?
                " Luckily, it contained the requested value\n" :
                " The result contained an error - Exiting program...\n");
        }
        if ((status_ & hasValue) == 0)
        {
            std::exit(EXIT_FAILURE);
        }

        return value_;
    }

    Error& GetErrorInternal()
    {
        if ((status_ & checked) == 0)
        {
            std::cerr << kResultCheckWarningMsg << (((status_ & hasValue) == 0) ?
                " Luckily, it contained the requested error\n" :
                " The result contained an error - Exiting program...\n");
        }
        if (status_ & hasValue)
        {
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

//template <>
//class Result<void>
//{
//public:
//    Result() : error_(), status_(hasValue) {}
//    Result(Error err) : error_(std::move(err)), status_(0) {}
//    ~Result() = default;
//
//    Result(Result&& other) noexcept : error_(std::move(other.error_)), status_(other.status_) {}
//
//    Result& operator=(Result&& other) noexcept
//    {
//        if (this == &other) { return *this; }
//
//        status_ = other.status_;
//        error_ = std::move(other.error_);
//
//        return *this;
//    }
//
//    Result(const Result&) = delete;
//    Result& operator=(const Result&) = delete;
//
//    bool Success() const
//    {
//        status_ |= checked;
//        return status_ & hasValue;
//    }
//
//    Error& GetError()
//    {
//        if ((status_ & checked) == 0)
//        {
//            std::cerr << kResultCheckWarningMsg << '\n';
//        }
//
//        return error_;
//    }
//    const Error& GetError() const
//    {
//        if ((status_ & checked) == 0)
//        {
//            std::cerr << kResultCheckWarningMsg << '\n';
//        }
//
//        return error_;
//    }
//
//private:
//    Error error_;
//
//    enum { hasValue = 1 << 0, checked = 1 << 1 };
//    mutable uint8_t status_;
//};
