#pragma once
#include "B2Joint.h"
#include "../core/WeightGenerator.h"


template <typename T>
class B2CompoundObject
{
public:
    B2CompoundObject() = default;
    explicit B2CompoundObject(std::vector<T>&& objs) : b2Objects_(std::move(objs)) {}

    size_t GetCount() const { return b2Objects_.size(); }

    std::vector<T>& GetB2Objects() { return b2Objects_; }
    const std::vector<T>& GetB2Objects() const { return b2Objects_; }

    T& operator[](size_t idx)
    {
        assert(idx < b2Objects_.size());
        return b2Objects_[idx];
    }
    const T& operator[](size_t idx) const
    {
        assert(idx < b2Objects_.size());
        return b2Objects_[idx];
    }

    T& Front()
    {
        assert(!b2Objects_.empty());
        return b2Objects_.front();
    }
    const T& Front() const { return Front(); }

    T& Back()
    {
        assert(!b2Objects_.empty());
        return b2Objects_.back();
    }
    const T& Back() const { return Back(); }

protected:
    template <typename...Args>
    using ValSetter = void(T::*)(Args...);

    template <typename Ret>
    using ValGetter = Ret(T::*)(void) const;

    template <typename Setter, typename...Args>
    void CascadeSetter(Setter setter, Range<size_t> span, Args&&...args)
    {
        for (size_t i = span.min; i < span.max; i++)
        {
            (b2Objects_[i].*setter)(std::forward<Args>(args)...);
        }
    }

    template <typename U>
    void ApplyWeightsInternal(ValGetter<U> getter, ValSetter<U> setter,
                              WeightGenerator& weights, std::optional<float> scaleFactor,
                              Range<size_t> span)
    {
        std::vector<U> objVals;
        objVals.reserve(span.max - span.min);

        for (size_t i = span.min; i < span.max; i++)
        {
            if (!b2Objects_[i].IsValid())
            {
                continue;
            }

            objVals.push_back((b2Objects_[i].*getter)());
        }

        weights.Apply(objVals, scaleFactor.value_or(objVals.size()));

        for (size_t i = span.min; i < span.max; i++)
        {
            if (!b2Objects_[i].IsValid())
            {
                continue;
            }

            (b2Objects_[i].*setter)(objVals[i]);
        }
    }

    Range<size_t> GetObjectSpan(const std::optional<Range<size_t>>& span)
    {
        Range<size_t> result{
            .min = 0,
            .max = b2Objects_.size() - 1
        };

        if (span.has_value())
        {
            auto [min, max] = *span;

            result.min = std::min(min, result.max);
            result.max = std::min(max, result.max);

            if (result.min > result.max)
            {
                std::swap(result.min, result.max);
            }
        }

        return result;
    }

private:
    std::vector<T> b2Objects_;
};

// COMPOUND JOINT
template <SomeDerivedB2Joint T>
class B2CompoundJoint : public B2CompoundObject<T>
{
public:
    B2CompoundJoint() = default;
    explicit B2CompoundJoint(std::vector<T>&& joints) : B2CompoundObject<T>(std::move(joints)) {}
};

class B2CompoundDistanceJoint : public B2CompoundJoint<B2DistanceJoint>
{
public:
    B2CompoundDistanceJoint() = default;
    explicit B2CompoundDistanceJoint(std::vector<B2DistanceJoint>&& joints) :
        B2CompoundJoint(std::move(joints)) {}

    void SetSpringHertz(WeightGenerator& weights, std::optional<float> scaleFactor = {},
                        std::optional<Range<size_t>> span = {})
    {
        ApplyWeightsInternal(&B2DistanceJoint::GetSpringHertz, &B2DistanceJoint::SetSpringHertz,
                             weights, scaleFactor, GetObjectSpan(span));
    }
    void SetSpringHertz(float hertz, std::optional<Range<size_t>> span = {})
    {
        CascadeSetter(&B2DistanceJoint::SetSpringHertz, GetObjectSpan(span), hertz);
    }

    void SetSpringDampingRatio(WeightGenerator& weights, std::optional<float> scaleFactor = {},
                               std::optional<Range<size_t>> span = {})
    {
        ApplyWeightsInternal(&B2DistanceJoint::GetSpringHertz, &B2DistanceJoint::SetSpringHertz,
                             weights, scaleFactor, GetObjectSpan(span));
    }
    void SetSpringDampingRatio(float damping, std::optional<Range<size_t>> span = {})
    {
        CascadeSetter(&B2DistanceJoint::SetSpringDampingRatio, GetObjectSpan(span), damping);
    }

    void SetMinLength(WeightGenerator& weights, std::optional<float> scaleFactor = {},
                      std::optional<Range<size_t>> span = {})
    {
        ApplyWeightsInternal(&B2DistanceJoint::GetMinLength, &B2DistanceJoint::SetMinLength,
                             weights, scaleFactor, GetObjectSpan(span));
    }
    void SetMinLength(float len, std::optional<Range<size_t>> span = {})
    {
        CascadeSetter(&B2DistanceJoint::SetMinLength, GetObjectSpan(span), len);
    }

    void SetMaxLength(WeightGenerator& weights, std::optional<float> scaleFactor = {},
                      std::optional<Range<size_t>> span = {})
    {
        ApplyWeightsInternal(&B2DistanceJoint::GetMaxLength, &B2DistanceJoint::SetMaxLength,
                             weights, scaleFactor, GetObjectSpan(span));
    }
    void SetMaxLength(float len, std::optional<Range<size_t>> span = {})
    {
        CascadeSetter(&B2DistanceJoint::SetMaxLength, GetObjectSpan(span), len);
    }

    void SetMotorSpeed(WeightGenerator& weights, std::optional<float> scaleFactor = {},
                       std::optional<Range<size_t>> span = {})
    {
        ApplyWeightsInternal(&B2DistanceJoint::GetMotorSpeed, &B2DistanceJoint::SetMotorSpeed,
                             weights, scaleFactor, GetObjectSpan(span));
    }
    void SetMotorSpeed(float speed, std::optional<Range<size_t>> span = {})
    {
        CascadeSetter(&B2DistanceJoint::SetMotorSpeed, GetObjectSpan(span), speed);
    }

    void SetSpringEnabled(bool enable, std::optional<Range<size_t>> span = {})
    {
        CascadeSetter(&B2DistanceJoint::SetSpringEnabled, GetObjectSpan(span), enable);
    }
    void SetMotorEnabled(bool enable, std::optional<Range<size_t>> span = {})
    {
        CascadeSetter(&B2DistanceJoint::SetMotorEnabled, GetObjectSpan(span), enable);
    }

private:

};