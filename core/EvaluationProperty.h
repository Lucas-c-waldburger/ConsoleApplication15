#pragma once
#include "TypeUtils.h"

enum class EvaluationSpec
{
    None,
    Constant,
    Scaled,
    Interpolated
};

template <typename T> 
    requires (HasPlusOperatorFor<T, T> && HasMinusOperatorFor<T, T> &&
             (HasMultOperatorFor<T, float> || std::convertible_to<T, float>))
struct EvaluationProperty
{
    EvaluationSpec spec = EvaluationSpec::None;
    T a;
    T b;

    static constexpr bool hasToFromFloat = (std::convertible_to<T, float> &&
                                            std::convertible_to<float, T>);

    T Evaluate(const T& val, float delta, float t) const
    {
        switch (spec)
        {
        case EvaluationSpec::Constant:
        {
            return val + a;
        }
        case EvaluationSpec::Scaled:
        {
            if constexpr (hasToFromFloat)
            {
                return static_cast<T>(static_cast<float>(val) +
                                      static_cast<float>(a) * delta);
            }
            else 
            {
                return val + a * delta;
            }
        }
        case EvaluationSpec::Interpolated:
        {
            if constexpr (hasToFromFloat)
            {
                const float bMinA = static_cast<float>(b - a);
                return static_cast<T>(static_cast<float>(a) + bMinA * t);
            }
            else
            {
                return a + (b - a) * t;
            }
        }
        case EvaluationSpec::None: default:
        {
            return val;
        }
        }
    }
};