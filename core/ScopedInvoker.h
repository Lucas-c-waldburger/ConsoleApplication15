#pragma once
#include <concepts>
#include <optional>

template <typename Fn> requires std::invocable<Fn>
class ScopedInvoker
{
public:
	explicit ScopedInvoker(Fn&& fn) : func_(std::forward<Fn>(fn)) {}
	~ScopedInvoker() { if (func_) { (*func_)(); } }

	ScopedInvoker(const ScopedInvoker&) = delete;
	ScopedInvoker& operator=(const ScopedInvoker&) = delete;

    ScopedInvoker(ScopedInvoker&& other) noexcept : func_(std::move(other.func_))
    {
        other.Release();
    }
    ScopedInvoker& operator=(ScopedInvoker&& other) noexcept 
    {
        if (this != &other) 
        {
            if (func_) { (*func_)(); }
            func_ = std::move(other.func_);
            other.Release();
        }
        return *this;
    }

	void Release() { func_.reset(); }

private:
	std::optional<Fn> func_;
};

