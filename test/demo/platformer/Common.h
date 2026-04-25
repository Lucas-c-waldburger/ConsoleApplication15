#pragma once
#include "../../../core/commonObjects.h"
#include "../../../core/SizedEnum.h"
#include "../../../systems/Phase.h"
#include "../../../components/TransformComponent.h"
#include "../../../components/RigidBodyComponent.h"
#include "../../../inputs/InputState.h"
#include "../../../systems/System.h"
#include "../../../deps/function2/function2.hpp"
#include "../../../components/UserComponents.h"
#include "../../../ecs/Ecs.h"
#include <array>

class Entity;

namespace test
{
static Dimensions<int> kGirlSpriteDimensions = { 100, 64 };
static Dimensions<float> kGirlIdleHitboxDimensions = { 26.0f, 59.0f };
static SDL_FPoint kGirlSpriteScale = { 1.8f, 1.8f };
static Dimensions<float> kGirlSwordHitboxDimensions = { 45.0f, 30.0f };
static SDL_FPoint kGirlSwordHitboxOffset = { 20.0f, 20.0f };

// GIRL
static constexpr std::string_view kGirlAttackASeriesName = "girl_attack_A";
static constexpr std::string_view kGirlAttackBSeriesName = "girl_attack_B";

static float kGirlColliderFriction = 0.8f;
static float kGirlColliderLandingFriction = 2.8f;

static float kGirlWalkStopVelocityX = 0.8f;

static float kGirlJumpImpulseY = 6.5f;
static float kGirlAccelGround = 45.0f;
static float kGirlAccelAir = 15.0f;
static float kGirlMaxWalkSpeedX = 6.0f;
static float kGirlDashImpulseX = 18.0f;

static float kGirlIdleAnimChangeTime = 0.35f;
static float kGirlLandAnimChangeTime = 0.14f;
static float kGirlLandAnimChangeTimeEndMod = 0.14f;
static float kGirlWalkAnimChangeXDelta = 13.0f;
static float kGirlJumpAnimChangeYDelta = 15.0f;
static float kGirlFallAnimChangeYDelta = 15.0f;
static float kGirlAttackAAnimChangeTime = 0.08f;
static float kGirlAttackBAnimChangeTime = 0.07f;
static float kGirlRollAnimChangeTime = 0.15f;
static float kGirlDashAnimChangeTime = 0.03f;
static float kGirlSheathAnimChangeTime = 0.14f;

static float kGirlDashAnimXDeltaDuration = 40.0f;
static float kGirlDashAnimYDeltaDuration = 20.0f;
static float kGirlDashAnimEnforcedTime = 0.25f;
static float kGirlDashCooldownTime = 0.65f;

static float kGirlAttackAltAnimWindowTime = 0.2f;
static float kGirlSheathSwordIdleTriggerTime = 2.0f;

static int kGirlControllerAxisDeadzone = 2700;

static Range<size_t> kGirlAttackAnimSwordActiveIdxRange = { 1, 2 };

static float kGirlNormalGravityScale = 2.0f;
static float kGirlJumpGravityScale = 3.5f;

// SWORD
static float kSwordHitImpulse = 80.5f;

struct ObjectCategory
{
	enum : uint8_t
	{
		Unknown = 0,
		Ground = 1 << 0,
		Wall = 1 << 1,
		Ceiling = 1 << 2,
		Enemy = 1 << 3,
		Player = 1 << 4,
		PlayerSword = 1 << 5,
		Terrain = (Ground | Wall | Ceiling)
	};

	using Type = uint8_t;
	static constexpr size_t count = 7;

	Type value = Unknown;
};

class CollisionCategoryTracker
{
public:
	constexpr uint16_t& operator[](ObjectCategory::Type category)
	{
		assert(category != ObjectCategory::Unknown);

		const auto idx = std::countr_zero(category);
		assert(idx < categories_.size());

		return categories_[static_cast<size_t>(idx)];
	}
	constexpr uint16_t operator[](ObjectCategory::Type category) const
	{
		assert(category != ObjectCategory::Unknown);

		const auto idx = std::countr_zero(category);
		assert(idx < categories_.size());

		return categories_[static_cast<size_t>(idx)];
	}

	constexpr void Increment(ObjectCategory::Type category)
	{
		if (category != ObjectCategory::Unknown)
		{
			const auto idx = std::countr_zero(category);
			assert(idx >= 0);
			assert(static_cast<size_t>(idx) < categories_.size());

			++categories_[static_cast<size_t>(idx)];
		}
	}

	constexpr void Decrement(ObjectCategory::Type category)
	{
		if (category != ObjectCategory::Unknown)
		{
			const auto idx = std::countr_zero(category);
			assert(idx >= 0);
			assert(static_cast<size_t>(idx) < categories_.size());

			if (categories_[static_cast<size_t>(idx)] > 0)
			{
				--categories_[static_cast<size_t>(idx)];
			}
		}
	}

	constexpr bool Empty() const
	{
		return std::apply([](const auto&...cats) {
			return ((cats == 0) && ...);
		}, categories_);
	}

	bool operator==(const CollisionCategoryTracker&) const = default;

private:
	std::array<uint16_t, ObjectCategory::count> categories_ = { 0 };
};

struct MoveTargets
{
	float targetVelX = 0.0f;
	float jumpVelY = 0.0f;
	float accelGround = 0.0f;
	float accelAir = 0.0f;
	float maxSpeed = 0.0f;
	float dt = 0.0f;
};

static const MoveTargets kGirlBaseMoveTargets{
	.targetVelX = 0.0f,
	.jumpVelY = kGirlJumpImpulseY,
	.accelGround = kGirlAccelGround,
	.accelAir = kGirlAccelAir,
	.maxSpeed = kGirlMaxWalkSpeedX
};

struct AnimationDeltas
{
	float idleTime = 0.0f;
	float attackATime = 0.0f;
	float attackBTime = 0.0f;
	float landTime = 0.0f;
	float landTimeEndMod = 0.0f;
	float walkDeltaX = 0.0f;
	float jumpDeltaY = 0.0f;
	float fallDeltaY = 0.0f;
	float sheathTime = 0.0f;
};

static const AnimationDeltas kGirlBaseAnimationDeltas{
	.idleTime = kGirlIdleAnimChangeTime,
	.attackATime = kGirlAttackAAnimChangeTime,
	.attackBTime = kGirlAttackBAnimChangeTime,
	.landTime = kGirlLandAnimChangeTime,
	.landTimeEndMod = kGirlLandAnimChangeTimeEndMod,
	.walkDeltaX = kGirlWalkAnimChangeXDelta,
	.jumpDeltaY = kGirlJumpAnimChangeYDelta,
	.fallDeltaY = kGirlFallAnimChangeYDelta,
	.sheathTime = kGirlSheathAnimChangeTime
};

struct MovementProfile
{
	struct GroundAir
	{
		float ground = 0.0f;
		float air = 0.0f;
	};
	struct HorizontalMovement
	{
		float maxSpeed = 0.0f;
		GroundAir acceleration;
		GroundAir deceleration;
	};
	struct Jump
	{
		float velocity = 0.0f;
		float cutMultiplier = 1.0f;
		float maxFallSpeed = 0.0f;
	};
	struct GravityScale
	{
		float rise = 1.0f;
		float fall = 1.0f;
	};
	struct Forgiveness
	{
		float coyoteTime = 0.0f;
		float jumpBufferTime = 0.0f;
	};
	struct AirControl
	{
		float multiplier = 1.0f;
		float turnBoost = 0.0f;
	};

	HorizontalMovement horiz;
	Jump jump;
	GravityScale gravScale;
	Forgiveness forgive;
	AirControl airControl;
	float groundSnapVelocity = 0.0f;
};

struct JumpTiming
{
	float coyoteTimer = 0.0f;
	float bufferTimer = 0.0f;
};

static constexpr MovementProfile kGirlMovementProfile{
	.horiz = {
		.maxSpeed = 9.0f,
		.acceleration = {
			.ground = 60.0f,
			.air = 35.0f
		},
		.deceleration = {
			.ground = 80.0f,
			.air = 15.0f
		}
	},
	.jump = {
		.velocity = 6.5f,
		.cutMultiplier = 0.5f,
		.maxFallSpeed = 20.0f
	},
	.gravScale = {
		.rise = 1.0f,
		.fall = 2.5f
	},
	.forgive = {
		.coyoteTime = 0.10f,
		.jumpBufferTime = 0.12f
	},
	.airControl = {
		.multiplier = 0.85f,
		.turnBoost = 1.2f
	},
	.groundSnapVelocity = 1.0f
};

static void ApplyHorizontalMovement(B2Body& body, float inputNormX,
									bool grounded, float dt,
									const MovementProfile& profile)
{
	float velX = body.GetLinearVelocity().x;

	const auto& horiz = profile.horiz;
	const auto& airControl = profile.airControl;

	float delta = 0.0f;
	float maxDelta = 0.0f;

	if (std::abs(inputNormX) < 0.01f)
	{
		float decel = grounded ? horiz.deceleration.ground : 
								 horiz.deceleration.air;

		delta = -velX;
		maxDelta = decel * dt;
	}
	else
	{
		float desired = inputNormX * horiz.maxSpeed;

		float accel = grounded ? horiz.acceleration.ground :
								 horiz.acceleration.air;
		if (!grounded)
		{
			accel *= airControl.multiplier;
		}

		// turn boost when reversing direction
		if (inputNormX != 0.0f && std::signbit(inputNormX) != std::signbit(velX))
		{
			accel *= airControl.turnBoost;
		}

		delta = desired - velX;
		maxDelta = accel * dt;
	}

	delta = std::clamp(delta, -maxDelta, maxDelta);

	float impulse = body.GetMass() * delta;

	body.ApplyLinearImpulseToCenter({ impulse, 0.0f });
}

static void ApplyJump(B2Body& body, const MovementProfile& profile)
{
	float impulse = body.GetMass() * profile.jump.velocity;

	body.ApplyLinearImpulseToCenter({ 0.0f, -impulse });
}

static void CutJump(B2Body& body, const MovementProfile& profile)
{
	auto vel = body.GetLinearVelocity();
	
	if (vel.y > 0.0f)
	{
		vel.y *= profile.jump.cutMultiplier;

		body.SetLinearVelocity(vel);
	}
}

static void ApplyGravityShape(B2Body& body, const MovementProfile& profile)
{
	float vy = body.GetLinearVelocity().y;

	float scale = (vy > 0.0f) ? profile.gravScale.rise
							  : profile.gravScale.fall;

	body.SetGravityScale(scale);
}

static void ClampFallSpeed(B2Body& body, const MovementProfile& profile)
{
	auto vel = body.GetLinearVelocity();

	if (vel.y > profile.jump.maxFallSpeed)
	{
		vel.y = profile.jump.maxFallSpeed;
		body.SetLinearVelocity(vel);
	}
}

static void UpdateGirlPhysics(
	RigidBody& rigid,
	float inputNormX,
	bool jumpPressed,
	bool jumpReleased,
	bool grounded,
	float dt,
	const MovementProfile& profile,
	JumpTiming& jumpTiming)
{
	auto& body = WriteAccessor<B2Body>{}(rigid.body);

	ApplyHorizontalMovement(body, inputNormX, grounded, dt, profile);

	ApplyGravityShape(body, profile);

	ClampFallSpeed(body, profile);

	// timers
	if (grounded)
	{
		jumpTiming.coyoteTimer = profile.forgive.coyoteTime;
	}
	else
	{
		jumpTiming.coyoteTimer -= dt;
	}

	if (jumpPressed)
	{
		jumpTiming.bufferTimer = profile.forgive.jumpBufferTime;
	}
	else
	{
		jumpTiming.bufferTimer -= dt;
	}

	// jump
	if (jumpTiming.coyoteTimer > 0 && jumpTiming.bufferTimer > 0)
	{
		ApplyJump(body, profile);
		jumpTiming.coyoteTimer = 0;
		jumpTiming.bufferTimer = 0;
	}

	if (jumpReleased)
	{
		CutJump(body, profile);
	}
}

struct GirlActionIntent
{
	InputState jumpIntent = InputState::None;
	InputState attackIntent = InputState::None;
	InputState dashIntent = InputState::None;
	std::optional<SDL_FPoint> moveIntent;

	void Reset() { *this = GirlActionIntent{}; }
};


struct GirlState
{
	enum Animation : uint32_t
	{
		Idle,
		Walking,
		Attacking,
		Jumping,
		Landing,
		Falling,
		Rolling,
		Dashing,
		Sheathing,
		ENUM_SIZE_
	};
	static constexpr size_t stateCount = enum_size_v<Animation>;
	static_assert(SomeSizedEnum<Animation>);

	Animation animation = Animation::Idle;
	CollisionCategoryTracker collidingCategories = {};
	GirlActionIntent action;
	//std::optional<int> axisMoveIntentX;

	bool operator==(const GirlState&) const = default;
};

static_assert(sizeof(GirlState) < kUserComponentStorageSize);

struct GirlIntent
{
	InputState jumpIntent = InputState::None;
	InputState attackIntent = InputState::None;
	SDL_FPoint moveIntent = { 0.0f, 0.0f };
};

struct GirlStateDeltas
{
	float deltaFrameTime = 0.0f;
	float deltaAnimTime = 0.0f;
	SDL_FPoint deltaAnimMove = { 0.0f, 0.0f };
	SDL_FPoint lastPosition = { 0.0f, 0.0f };
};

enum class GirlStateEnum : uint16_t
{
	Idle,
	Walking,
	Attacking,
	Jumping,
	Landing,
	Falling,
	ENUM_SIZE_
};

struct EvaluatedGirlStateContext
{
	bool onGround = false;
	bool moved = false;
	bool jumpPressed = false;
	bool attackPressed = false;
	bool thumbstickEngaged = false;
};

struct GirlStateContextOld
{
	enum Flag : uint8_t
	{
		PerformJumpPressedIntent = 1 << 0,
		PerformLandingIntent = 1 << 1
	};

	MoveTargets targets = kGirlBaseMoveTargets;
	GirlStateDeltas deltas;
	//GirlIntent intents;
	//CollisionCategoryTracker collidingCategories;
	uint8_t flags = 0;

	static EvaluatedGirlStateContext Evaluate(const Entity& girl, const GirlStateContextOld& ctx);
};

struct GirlStateChangeEvent
{
	GirlState::Animation lastState = GirlState::Animation::Idle;
	GirlState::Animation newState = GirlState::Animation::Idle;
};

static float ComputeMoveImpulseX(const RigidBody& rigid, const GirlState& state,
								 const MoveTargets& targets, float desiredVelX, float dt)
{
	const auto& body = rigid.body.GetData();

	float velX = body.GetLinearVelocity().x;

	float delta = desiredVelX - velX;

	float accel = state.collidingCategories[ObjectCategory::Ground] > 0
		? targets.accelGround
		: targets.accelAir;

	// limit how much velocity we change this frame
	float maxDelta = accel * dt;

	delta = std::clamp(delta, -maxDelta, maxDelta);

	// impulse = mass * velocity change
	return body.GetMass() * delta;
}

static float ComputeJumpImpulseY(const RigidBody& rigid, float jumpVelY)
{
	const auto& body = rigid.body.GetData();

	float currentVelY = body.GetLinearVelocity().y;

	float delta = jumpVelY - currentVelY;

	return body.GetMass() * delta;
}

static SDL_FPoint ComputeDashImpulse(const RigidBody& rigid, SDL_FPoint dashVel)
{
	const auto& body = rigid.body.GetData();

	SDL_FPoint currentVel = body.GetLinearVelocity();

	SDL_FPoint delta = dashVel - currentVel;

	return delta * body.GetMass();
}

struct GirlStateContext
{
	Entity girl;
	SDL_FPoint lastPosition = { 0.0f, 0.0f };
	float timeInCurrentAnimFrame = 0.0f;
	SDL_FPoint distanceInCurrentAnimFrame = { 0.0f, 0.0f };
	float dt = 0.0f;
};

struct CollisionCache
{
	std::unordered_map<Handle<B2Shape>, Entity_t> data;

	CollisionCache() = default;
	~CollisionCache() = default;
	CollisionCache(CollisionCache&) = default;
	CollisionCache& operator=(CollisionCache&) = default;
	CollisionCache(CollisionCache&&) noexcept = default;
	CollisionCache& operator=(CollisionCache&&) noexcept = default;
};

inline constexpr float Dot(SDL_FPoint a, SDL_FPoint b)
{
	return a.x * b.x + a.y * b.y;
}

inline SDL_FPoint Normalize(SDL_FPoint ax)
{
    float mag = std::sqrt(ax.x * ax.x + ax.y + ax.y);
    if (mag > 0.0001f)
    {
        return { ax.x / mag, ax.y / mag };
    }
    return { 0.0f, -1.0f };
}

inline constexpr SDL_FPoint MulSV(float s, SDL_FPoint v)
{
	return { s * v.x, s * v.y };
}

inline SDL_FPoint GetGirlSpawnPosition()
{
	const auto [winCenterX, winCenterY] =
		SDLite::Window().GetLocalCenter<SDL_FPoint>();

	return { winCenterX, winCenterY - 60.0f };
}

inline bool OnTopOfAShape(const Collider& collider)
{
	const auto& shape = collider.shape.GetData();

	if (!shape.IsValid())
	{
		return false;
	}

	static constexpr SDL_FPoint up = { 0.0f, -1.0f };

	auto contacts = shape.GetContactData();
	for (const auto& contact : contacts)
	{
		auto normal = contact.manifold.normal;
		if (contact.shapeHandleA == shape.GetHandle())
		{
			normal = MulSV(-1.0f, normal);
		}

		float alignment = Dot(normal, up);

		if (alignment > 0.7f)
		{
			return true;
		}
	}

	return false;
}

inline SDL_FPoint GetAxisNormed(SDL_FPoint axis)
{
	SDL_FPoint dir = {
		axis.x / 32768.0f,
		axis.y / 32768.0f
	};

	// Deadzone
	float lenSq = dir.x * dir.x + dir.y * dir.y;
	if (lenSq < 0.01f)
	{
		return { 0.0f, 0.0f };
	}

	// Normalize
	float invLen = 1.0f / std::sqrt(lenSq);

	return { dir.x * invLen, dir.y * invLen };
}

class EntityMap : public UnorderedDictionary<Entity>
{
public:
	bool AllValid() const 
	{ 
		return std::all_of(this->begin(), this->end(), [](const auto& pair) {
			return pair.second.IsValid();
		});
	}

private:
};

//template <GirlState::Animation PrimaryState, typename SubStateEnum>
//	requires std::is_enum_v<SubStateEnum>
//struct GirlStateUnion
//{
//public:
//	static constexpr GirlState::Animation primaryState = PrimaryState;
//	SubStateEnum subState;
//};
//
//template <typename T>
//struct is_girl_state_union : std::false_type {};
//
//template <GirlState::Animation PrimaryState, typename SubStateEnum>
//struct is_girl_state_union<GirlStateUnion<PrimaryState, SubStateEnum>> : std::true_type {};
//
//enum class GirlAttackSubState
//{
//	AttackA,
//	AttackB
//};

//template <GirlState::Animation stateVal>
//class EmptyGirlStateHandlerImpl
//{
//	friend class Super;
//
//	void UpdateImpl(GirlStateContext&) {}
//	void OnEnterImpl(GirlStateContext&) {}
//	void OnExitImpl(GirlStateContext&) {}
//};
//
//template <GirlState::Animation stateVal, typename Derived = EmptyGirlStateHandler<stateVal>>
//class GirlStateHandler
//{
//public:
//	using Super = GirlStateHandler<stateVal, Derived>;
//	using State = GirlState::Animation;
//
//	static constexpr State state = stateVal;
//	using DerivedType = Derived;
//
//	void Update(GirlStateContext& ctx)
//	{
//		return static_cast<Derived*>(this)->UpdateImpl(ctx);
//	}
//
//	void OnEnter(GirlStateContext& ctx)
//	{
//		return static_cast<Derived*>(this)->OnEnterImpl(ctx);
//	}
//
//	void OnExit(GirlStateContext& ctx)
//	{
//		return static_cast<Derived*>(this)->OnExitImpl(ctx);
//	}
//};
//
//template <GirlState::Animation stateVal>
//class EmptyGirlStateHandler : public GirlStateHandler<stateVal>
//{};
//
//template <typename T>
//concept SomeGirlStateHandler = requires {
//	std::same_as<std::remove_cvref_t<decltype(T::state)>, GirlState::Animation>;
//	std::derived_from<T, GirlStateHandler<T::state, T>>;
//};
//
//template <typename A, typename B>
//struct sort_by_girl_state_enum_val : 
//	std::bool_constant<(static_cast<size_t>(A::state) < static_cast<size_t>(B::state))> {};
//
//template <SomeGirlStateHandler...Ts> requires pack_types_unique_v<Ts...>
//struct girl_state_handler_tuple
//{
//	using list = TypeList<Ts...>;
//	using type = sort_types_t<list, sort_by_girl_state_enum_val>::AsTuple<std::type_identity>;
//};
//
//template <typename...Ts>
//using girl_state_handler_tuple_t = girl_state_handler_tuple<Ts...>::type;

//template <typename TupLike, typename ISeq>
//struct girl_state_handler_tuple_impl;
//
//template <template <typename...> class TupLike, typename...Ts, size_t...Is>
//struct girl_state_handler_tuple_impl<TupLike<Ts...>, std::index_sequence<Is...>>
//{
//
//	using type = std::tuple<typename Ts::DerivedType...>;
//};



//class GirlStateHandlers
//{
//public:
//	using enum GirlState::Animation;
//
//	using AttackHandlerType = EmptyGirlStateHandler<Attacking>;
//	using WalkHandlerType	= EmptyGirlStateHandler<Walking>;
//	using JumpHandlerType	= EmptyGirlStateHandler<Jumping>;
//	using FallHandlerType	= EmptyGirlStateHandler<Falling>;
//	using LandHandlerType	= EmptyGirlStateHandler<Landing>;
//	using IdleHandlerType	= EmptyGirlStateHandler<Idle>;
//
//private:
//	using HandlerTuple = girl_state_handler_tuple_t<
//		AttackHandlerType,
//		WalkHandlerType,
//		JumpHandlerType,
//		FallHandlerType,
//		LandHandlerType,
//		IdleHandlerType
//	>;
//	
//	HandlerTuple handlers_;
//
//public:
//	template <GirlState::Animation enumVal> requires (static_cast<size_t>(enumVal) < 
//													  std::tuple_size_v<HandlerTuple>)
//	auto& GetHandler()
//	{
//		return std::get<static_cast<size_t>(enumVal)>(handlers_);
//	}
//
//
//};

//template <GirlState::Animation PrimaryState, typename SubStateEnum, typename DataCtx=Void>
//class GirlStateHandler : GirlStateUnion<PrimaryState, SubStateEnum>
//{
//public:
//	
//
//protected:
//	DataCtx dataContext_;
//};

//template <GirlState::Animation>
//struct GirlStateHandler {};
//
//template <typename Handler>
//struct get_girl_state_enum_val;
//
//template <GirlState::Animation enumVal>
//struct get_girl_state_enum_val<GirlStateHandler<enumVal>>
//{
//	static constexpr GirlState::Animation value = enumVal;
//};

//template <typename Handler>
//concept SomeGirlStateHandler = requires(Handler& h, Entity& e) {
//	{ h.onEnter(e) } -> std::same_as<void>;
//	{ h.onExit(e) } -> std::same_as<void>;
//	{ h.onExit(e) } -> std::same_as<void>;
//};


//template <SomeSizedEnum E>
//using sized_enum_iseq = std::make_index_sequence<enum_size_v<E>>;
//
//template <typename Derived, GirlState::Animation, typename ISeq>
//struct girl_state_tuple_impl;



//
//template <template <GirlState::Animation> class Wrap, typename ISeq>
//struct girl_state_tuple_impl;
//
//template <template <GirlState::Animation> class Wrap, size_t...Is>
//struct girl_state_tuple_impl<Wrap, std::index_sequence<Is...>>
//{
//	using type = std::tuple<Wrap<static_cast<GirlState::Animation>(Is)>...>;
//};
//
//template <template <GirlState::Animation> class Wrap>
//struct girl_state_tuple : girl_state_tuple_impl<Wrap, sized_enum_iseq<GirlState::Animation>> {};
//
//template <template <GirlState::Animation> class Wrap>
//using girl_state_tuple_t = girl_state_tuple<Wrap>::type;
//
//class GirlStateHandlers
//{
//public:
//	template <GirlState::Animation stateVal>
//	GirlStateHandler<stateVal>& GetStateHandler() 
//	{ 
//		return std::get<GirlStateHandler<stateVal>>(handlers_);
//	}
//
//private:
//	using HandlerTuple = girl_state_tuple_t<GirlStateHandler>;
//
//	HandlerTuple handlers_;
//};


//class GirlAttackStateHandler : GirlStateUnion<GirlState::Animation::Attacking,
//											  GirlAttackSubState>
//{
//public:
//
//private:
//};


//static ObjectCategory::Type GetEntityObjectCategory(Entity_t entityId)
//{
//	auto e = ECS::GetEntityByID(entityId);
//	if (!e.IsValid())
//	{
//		return ObjectCategory::Unknown;
//	}
//
//	return e.HasComponent<ObjectCategory>()
//		? e.GetComponent<ObjectCategory>().value
//		: ObjectCategory::Unknown;
//}

//template <Phase ph, size_t ord = 0>
//struct SystemUpdateFunction
//{
//	using Signature = void(*)(float);
//
//	static constexpr Phase phase = ph;
//	static constexpr size_t order = ord;
//
//	SystemUpdateFunction() = default;
//	template <typename Fn> requires std::convertible_to<Fn, Signature>
//	explicit SystemUpdateFunction(Fn&& fn) : fnPtr(std::forward<Fn>(fn)) {}
//
//	void Update(float dt) { if (fnPtr) { std::invoke(fnPtr, dt); } }
//
//	Signature fnPtr = nullptr;
//};
//
//template <std::underlying_type_t<Phase>...Is> 
//using PhaseIndexSequence = std::integer_sequence<std::underlying_type_t<Phase>, Is...>;
//
//using MakePhaseIndexSequence = 
//	std::make_integer_sequence<std::underlying_type_t<Phase>, enum_size_v<Phase>>;
//
//
//template <typename T, Phase ph, size_t...Is>
//inline constexpr bool has_sys_update_for_phase_v = 
//	(std::derived_from<T, SystemUpdateFunction<ph, Is>> || ...);
//
//template <typename T, typename PhaseISeq, typename ISeq>
//struct derives_sys_update_fns;
//
//template <typename T, std::underlying_type_t<Phase>...phIs, size_t...Is>
//struct derives_sys_update_fns<T, PhaseIndexSequence<phIs...>, std::index_sequence<Is...>>
//{
//	static constexpr bool value = (
//		has_sys_update_for_phase_v<T, static_cast<Phase>(phIs), Is...> || ...
//	);
//};
//
//inline constexpr size_t kMaxSystemUpdateFnOrderValue = 10;
//
//using MakeSysUpdateOrderIndexSequence = std::make_index_sequence<kMaxSystemUpdateFnOrderValue>;
//
//template <typename T>
//concept DerivesSystemUpdateFunctions =
//	derives_sys_update_fns<T, MakePhaseIndexSequence, MakeSysUpdateOrderIndexSequence>::value;
//
//template <Phase ph, size_t...Is>
//using sys_update_types_for_phase_t = TypeList<SystemUpdateFunction<ph, Is>...>;
//
//template <typename PhaseISeq, typename ISeq>
//struct all_possible_sys_update_types;
//
//template <std::underlying_type_t<Phase>...phIs, size_t...Is>
//struct all_possible_sys_update_types<PhaseIndexSequence<phIs...>, std::index_sequence<Is...>>
//{
//	using type = concat_type_lists_t<
//		sys_update_types_for_phase_t<static_cast<Phase>(phIs), Is...>...
//	>;
//};
//
//
//using all_possible_sys_update_types_t =
//	all_possible_sys_update_types<MakePhaseIndexSequence, MakeSysUpdateOrderIndexSequence>::type;
//
//template <typename T>
//struct extract_derived_sys_update_types
//{
//	template <typename U>
//	using derives_from = std::is_base_of<U, T>;
//
//	using type = filter_types_t<
//		all_possible_sys_update_types_t,
//		derives_from
//	>;
//
//};
//
//template <typename T>
//using extract_derived_sys_update_types_t = 
//	extract_derived_sys_update_types<T>::type;
//
//
//template <typename...Ts>
//struct TestSystemManager
//{
//	using Order = size_t;
//
//	std::array<std::vector<fu2::unique_function<void(float)>>,
//		enum_size_v<Phase>> data;
//
//	std::tuple<Ts...> providers;
//	std::array<std::vector<Order>, enum_size_v<Phase>> ordering;
//
//	template <typename T, typename...Args> 
//		requires std::constructible_from<T, Args...>
//	void RegisterProvider(Args&&...args)
//	{
//		auto& provider = std::get<T>(providers);
//		provider = T{ std::forward<Args>(args)... };
//
//		using Updates = extract_derived_sys_update_types_t<T>;
//
//		static constexpr auto doOne = []<size_t I>
//		(TestSystemManager& manager, T& prov) {
//			using Fn = type_at_index_t<I, Updates>;
//
//			manager.template RegisterImpl<Fn::phase, Fn::order>(prov);
//		};
//
//		[]<size_t...Is>(TestSystemManager& manager, T& prov, std::index_sequence<Is...>) 
//		{
//			((doOne.template operator()<Is>(manager, prov)), ...);
//		}(*this, provider, std::make_index_sequence<Updates::size>{});
//	}
//
//	template <Phase ph, size_t ord, typename T>
//	static fu2::unique_function<void(float)> GetLambda(T& provider) {
//		return [&provider](float dt) {
//			LOG_DEBUG_FMT("Running Update Order {}", ord);
//			provider.SystemUpdateFunction<ph, ord>::Update(dt);
//		};
//	}
//
//	template <Phase ph, size_t ord, typename T>
//	void RegisterImpl(T& provider)
//	{
//		auto& ords = ordering[static_cast<size_t>(ph)];
//		auto& v = data[static_cast<size_t>(ph)];
//		assert(ords.size() == v.size());
//
//		if (ords.empty() || ord >= ords.back())
//		{
//			ords.emplace_back(ord);
//			v.emplace_back(GetLambda<ph, ord>(provider));
//
//			return;
//		}
//		// else need to insert it in the proper place by order
//
//		// find spot to insert inside ords
//		auto it = std::lower_bound(ords.begin(), ords.end(), ord);
//		if (it == ords.end())
//		{
//			ords.emplace_back(ord);
//			v.emplace_back(GetLambda<ph, ord>(provider));
//
//			return;
//		}
//
//		// if already have one with the same ord, move it to the 
//		// pos of the last matching one, or do nothing if first instance of ord
//		while (*it == ord)
//		{
//			++it;
//		}
//			
//		// grab the index it should go inside v
//		const size_t idx = std::distance(ords.begin(), it);
//		assert(idx < v.size());
//
//		ords.insert(it, ord);
//
//		// use idx to find iter position inside v
//		auto vPos = v.begin() + idx;
//
//		v.insert(vPos, GetLambda<ph, ord>(provider));
//	}
//
//	void RunSystemUpdates(Phase phase, float dt)
//	{
//		auto& v = data[static_cast<size_t>(phase)];
//
//		for (auto& fn : v)
//		{
//			assert(fn);
//			std::invoke(fn, dt);
//		}
//	}
//};


} // test