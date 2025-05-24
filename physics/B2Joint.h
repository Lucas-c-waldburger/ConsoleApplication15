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

	void Destroy() 
	{ 
		b2DestroyJoint(jointHandle_);
		jointHandle_ = {};
	}

	Handle<B2Body> GetBodyHandleA() const { return Handle<B2Body>::Create(b2Joint_GetBodyA(jointHandle_)); }
	Handle<B2Body> GetBodyHandleB() const { return Handle<B2Body>::Create(b2Joint_GetBodyB(jointHandle_)); }

	std::pair<SDL_FPoint, SDL_FPoint> GetEndPoints() const
	{
		b2Vec2 localAnchorA = b2Joint_GetLocalAnchorA(jointHandle_);
		b2Vec2 localAnchorB = b2Joint_GetLocalAnchorB(jointHandle_);

		b2Transform tfA = b2Body_GetTransform(GetBodyHandleA());
		b2Transform tfB = b2Body_GetTransform(GetBodyHandleB());

		b2Vec2 worldPointA = b2TransformPoint(tfA, localAnchorA);
		b2Vec2 worldPointB = b2TransformPoint(tfB, localAnchorB);

		return std::make_pair(
			ToSDLFPointScaled(worldPointA), ToSDLFPointScaled(worldPointB)
		);
	}

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

	float GetMinLength() const { return b2DistanceJoint_GetMinLength(jointHandle_); }
	void SetMinLength(float newMin) 
	{ 
		b2DistanceJoint_SetLengthRange(jointHandle_, newMin, b2DistanceJoint_GetMaxLength(jointHandle_)); 
	}

	float GetMaxLength() const { return b2DistanceJoint_GetMaxLength(jointHandle_); }
	void SetMaxLength(float newMax)
	{
		b2DistanceJoint_SetLengthRange(jointHandle_, b2DistanceJoint_GetMinLength(jointHandle_), newMax);
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

template <SomeDerivedB2Joint T>
struct B2JointParams;

template <>
struct B2JointParams<B2DistanceJoint>
{
	struct
	{
		std::optional<float> rest;
		std::optional<float> min;
		std::optional<float> max;
	} length;

	struct
	{
		SDL_FPoint a = { 0.0f, 0.0f };
		SDL_FPoint b = { 0.0f, 0.0f };
	} localAnchor;

	struct 
	{
		bool enable = false;
		std::optional<float> hertz;
		std::optional<float> dampingRatio;
		std::optional<bool> limit;
	} spring;

	struct
	{
		bool enable = false;
		std::optional<float> maxForce;
		std::optional<float> speed;
	} motor;

	bool collideConnected = false;
};

class B2JointFactory
{
public:
	static B2DistanceJoint MakeDistanceJoint(const Handle<B2Body>& bodyA, const Handle<B2Body>& bodyB,
											 const B2JointParams<B2DistanceJoint>& params)
	{
		if (!(bodyA.IsValid() && bodyB.IsValid()))
		{
			return {};
		}

		b2WorldId bodyAWorld = b2Body_GetWorld(bodyA);
		b2WorldId bodyBWorld = b2Body_GetWorld(bodyB);

		if (!(b2World_IsValid(bodyAWorld) && b2World_IsValid(bodyBWorld)) || bodyAWorld != bodyBWorld)
		{
			return {};
		}

		b2DistanceJointDef def = b2DefaultDistanceJointDef();

		def.bodyIdA = bodyA;
		def.bodyIdB = bodyB;

		const auto& len = params.length;
		if (!len.rest.has_value())
		{
			def.length = b2Distance(b2Body_GetPosition(bodyA), b2Body_GetPosition(bodyB));
		}
		else
		{
			def.length = *len.rest;
		}

		auto trySet = [](const auto& op, auto& defMember) -> void {
			if (op.has_value()) { defMember = *op; }
		};
		auto anyHaveValue = [](const auto&...ops) -> bool {
			return (ops.has_value() || ...);
		};

		trySet(len.min, def.minLength);
		trySet(len.max, def.maxLength);

		def.localAnchorA = ToB2VecScaled(params.localAnchor.a);
		def.localAnchorB = ToB2VecScaled(params.localAnchor.b);

		const auto& spring = params.spring;
		if (spring.enable || anyHaveValue(spring.hertz, spring.dampingRatio, spring.limit))
		{
			def.enableSpring = true;
			trySet(spring.hertz, def.hertz);
			trySet(spring.dampingRatio, def.dampingRatio);
			trySet(spring.limit, def.enableLimit);
		}

		const auto& motor = params.motor;
		if (motor.enable || anyHaveValue(motor.maxForce, motor.speed))
		{
			def.enableMotor = true;
			trySet(motor.maxForce, def.maxMotorForce);
			trySet(motor.speed, def.motorSpeed);
		}

		def.collideConnected = params.collideConnected;
		
		auto handle = Handle<B2Joint>::Create(b2CreateDistanceJoint(bodyAWorld, &def));

		return B2DistanceJoint{ handle };
	}

private:
	B2JointFactory() = default;
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
