#pragma once
#include "TypeInfo.h"
#include "TypeUtils.h"
#include "EnumUtils.h"

enum class TypeQualifier : uint8_t
{
	Const = 1 << 0,
	Ref = 1 << 1,
	Ptr = 1 << 2,
	ConstPtr = 1 << 3
};

template <> inline constexpr bool enable_bitwise_operators_v<TypeQualifier> = true;

namespace detail {

// reject ptr to ptr, ptr to array, ptr to function, array, function, rvalue reference
template <typename T>
concept QualifiedTypeIdentityCompliant =
    !std::is_rvalue_reference_v<T> &&
    !std::is_pointer_v<std::remove_pointer_t<std::remove_reference_t<T>>> &&
    !std::is_array_v<std::remove_reference_t<T>> &&
    !std::is_function_v<std::remove_reference_t<T>> &&
    (!std::is_pointer_v<std::remove_reference_t<T>> ||
     (!std::is_array_v<std::remove_pointer_t<std::remove_reference_t<T>>> &&
      !std::is_function_v<std::remove_pointer_t<std::remove_reference_t<T>>>));

} // detail

template <detail::QualifiedTypeIdentityCompliant T>
consteval TypeQualifier GetTypeQualifier() noexcept
{
    using NoRef = std::remove_reference_t<T>;

    TypeQualifier qualifier = static_cast<TypeQualifier>(0);

    if constexpr (std::is_reference_v<T>)
    {
        qualifier |= TypeQualifier::Ref;
    }
    if constexpr (std::is_pointer_v<NoRef>)
    {
        if constexpr (std::is_const_v<NoRef>)
        {
            qualifier |= TypeQualifier::ConstPtr;
        }           
        else
        {
            qualifier |= TypeQualifier::Ptr;
        }

        if constexpr (std::is_const_v<std::remove_pointer_t<NoRef>>)
        {
			qualifier |= TypeQualifier::Const;
        }
    }
    else if constexpr (std::is_const_v<NoRef>)
    {
        qualifier |= TypeQualifier::Const;
    }

    return qualifier;
}

struct QualifiedTypeIdentity
{
	uint64_t value = 0;

	template <detail::QualifiedTypeIdentityCompliant T>
    static constexpr QualifiedTypeIdentity Create() noexcept
    {
        using RawType = raw_type_t<T>;

        return {
            .value = { (static_cast<uint64_t>(GetTypeQualifier<T>()) << 32) | 
                        static_cast<uint64_t>(TypeInfo<RawType>::hash32) }
        };
	}

    constexpr uint32_t GetRawType() const noexcept
    {
        return static_cast<uint32_t>(value & 0xFFFFFFFF);
	}
    constexpr bool IsConst() const noexcept
    {
        return (static_cast<TypeQualifier>((value >> 32) & 0xFF) & TypeQualifier::Const) 
                != static_cast<TypeQualifier>(0);
	}
    constexpr bool IsRef() const noexcept
    {
        return (static_cast<TypeQualifier>((value >> 32) & 0xFF) & TypeQualifier::Ref) 
                != static_cast<TypeQualifier>(0);
    }
    constexpr bool IsPtr() const noexcept
    {
        return (static_cast<TypeQualifier>((value >> 32) & 0xFF) & TypeQualifier::Ptr) 
                != static_cast<TypeQualifier>(0);
    }
    constexpr bool IsConstPtr() const noexcept
    {
        return (static_cast<TypeQualifier>((value >> 32) & 0xFF) & TypeQualifier::ConstPtr) 
                != static_cast<TypeQualifier>(0);
	}
    constexpr bool IsValue() const noexcept
    {
        return !IsRef() && !IsPtr() && !IsConstPtr();
	}
};