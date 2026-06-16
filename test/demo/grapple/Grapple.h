#pragma once
#include "../../../ecs/Ecs.h"
#include "../../../physics/B2Joint.h"
#include "../../../physics/B2World.h"
#include "../../../atlas/NewTextureRepository.h"


namespace test {

class LegIron
{
public:
	struct Definition
	{
		SDL_FPoint headPos = { 0.0f, 0.0f };
		size_t numLinks = 0;
		uint8_t playerCategory = 0;
		uint8_t legIronCategory = 0;
		uint8_t ballSensorCategory = 0;
	};

	static Result<LegIron> Create(B2World& world, TextureRepository& repo,
								  Entity& connectingEnt, const Definition& def);

	std::vector<SDL_FPoint> GetPoints() const;

	Entity GetBallEntity();
	const Entity GetBallEntity() const;
	Entity GetBallSensor();

	template <typename Fn> requires std::invocable<Fn, B2DistanceJoint&>
	void ForEachJoint(Fn&& fn)
	{
		for (auto& j : joints_) { std::invoke(fn, j); }
	}

	float GetDefaultJointLength() const noexcept { return defaultJointLength_; }
	void SetJointRestLength(float len);
	float GetJointRestLength() const noexcept;

private:
	std::vector<Entity> entities_;
	std::vector<B2DistanceJoint> joints_;
	float defaultJointLength_ = 0.0f;
};

class Player
{

};

//class GrappleDrawSystem
//{
//public:
//	explicit GrappleDrawSystem(GrappleRope& rope) : rope_(&rope) {}
//	void Update(float);
//
//private:
//	GrappleRope* rope_ = nullptr;
//};

} // test