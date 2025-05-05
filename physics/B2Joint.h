#pragma once
#include "B2Body.h"



class B2Joint
{
public:
	B2Joint() = default;

	enum class Type
	{
		Distance = b2_distanceJoint,
		Filter = b2_filterJoint,
		Motor = b2_motorJoint,
		Mouse = b2_mouseJoint,
		Prismatic = b2_prismaticJoint,
		Revolute = b2_revoluteJoint,
		Weld = b2_weldJoint,
		Wheel = b2_wheelJoint
	};

	Type GetJointType() const { return static_cast<Type>(b2Joint_GetType(jointHandle_)); }

	bool IsValid() const { return jointHandle_.IsValid(); }

	Handle<B2Body> GetBodyHandleA() const { return Handle<B2Body>::Create(b2Joint_GetBodyA(jointHandle_)); }
	Handle<B2Body> GetBodyHandleB() const { return Handle<B2Body>::Create(b2Joint_GetBodyB(jointHandle_)); }

	template <typename T>
	T GetAs();
	
protected:
	explicit B2Joint(const Handle<B2Joint>& handle) : jointHandle_(handle) {}

	Handle<B2Joint> jointHandle_;
};

template <typename T>
concept SomeDerivedB2Joint = std::derived_from<T, B2Joint>&&
							 std::constructible_from<T, const Handle<B2Joint>&>&&
							 std::same_as<std::remove_cvref_t<decltype(T::jointType)>, B2Joint::Type>;

template <SomeDerivedB2Joint T>
inline bool JointTypeMatches(const Handle<B2Joint>& jointHandle)
{
	return T::jointType == static_cast<B2Joint::Type>(b2Joint_GetType(jointHandle));
}

template <SomeDerivedB2Joint T>
struct B2JointDefinition;


class B2DistanceJoint : public B2Joint
{
public:
	static constexpr B2Joint::Type jointType = B2Joint::Type::Distance;

	B2DistanceJoint() = default;
	explicit B2DistanceJoint(const Handle<B2Joint>& handle) : B2Joint(handle) {}

	float GetCurrentLength() const { return b2DistanceJoint_GetCurrentLength(jointHandle_); }

	float GetRestLength() const { return b2DistanceJoint_GetLength(jointHandle_); }
	void SetRestLength(float newLen) { b2DistanceJoint_SetLength(jointHandle_, newLen); }

	Range<float> GetLengthRange() const
	{
		return {
			.min = b2DistanceJoint_GetMinLength(jointHandle_),
			.max = b2DistanceJoint_GetMaxLength(jointHandle_)
		};
	}
	void SetLengthRange(Range<float> range) { b2DistanceJoint_SetLengthRange(jointHandle_, range.min, range.max); }

	bool IsSpringEnabled() const { return b2DistanceJoint_IsSpringEnabled(jointHandle_); }
	void SetSpringEnabled(bool enable) { b2DistanceJoint_EnableSpring(jointHandle_, enable); }

	float GetSpringHertz() const { return b2DistanceJoint_GetSpringHertz(jointHandle_); }
	void SetSpringHertz(float newHz) { b2DistanceJoint_SetSpringHertz(jointHandle_, newHz); }

	float GetSpringDampingRatio() const { return b2DistanceJoint_GetSpringDampingRatio(jointHandle_); }
	void SetSpringDampingRatio(float newRatio) { b2DistanceJoint_SetSpringDampingRatio(jointHandle_, newRatio); }

	bool IsSpringLimitEnabled() const { return b2DistanceJoint_IsLimitEnabled(jointHandle_); }
	void SetSpringLimitEnabled(bool enable) { b2DistanceJoint_EnableLimit(jointHandle_, enable); }

	void SetMotorEnabled(bool enable) { b2DistanceJoint_EnableMotor(jointHandle_, enable); }
	bool IsMotorEnabled() const { return b2DistanceJoint_IsMotorEnabled(jointHandle_); }

	float GetMotorSpeed() const { return b2DistanceJoint_GetMotorSpeed(jointHandle_); }
	void SetMotorSpeed(float newSpeed) { b2DistanceJoint_SetMotorSpeed(jointHandle_, newSpeed); }

	float GetMotorForce() const { return b2DistanceJoint_GetMotorForce(jointHandle_); };

	float GetMaxMotorForce() const { return b2DistanceJoint_GetMaxMotorForce(jointHandle_); }
	void SetMaxMotorForce(float newForce) { b2DistanceJoint_SetMaxMotorForce(jointHandle_, newForce); }



private:
};

template <>
struct B2JointDefinition<B2DistanceJoint>
{
	B2JointDefinition() : jointDef(b2DefaultDistanceJointDef()) {}
	b2DistanceJointDef jointDef;
};


template<typename T>
inline T B2Joint::GetAs()
{
	static_assert(SomeDerivedB2Joint<T>);

	if (JointTypeMatches<T>(jointHandle_))
	{
		return T{ jointHandle_ };
	}

	return T{};
}
