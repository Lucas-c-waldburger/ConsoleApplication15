#pragma once
#include "B2Body.h"
#include "B2Utils.h"
#include <set>
#include <functional>

struct B2RayCastReturn
{
	explicit constexpr B2RayCastReturn(float f) noexcept : fraction_(f) {}

	constexpr float Ignore() { return -1.0f; }
	constexpr float Terminate() { return 0.0f; }
	constexpr float Clip() { return fraction_; }
	constexpr float Continue() { return 1.0f; }

private:
	float fraction_;
};

struct B2RayCastResult
{
	Handle<B2Shape> shapeHandle;
	SDL_FPoint point;
	SDL_FPoint normal;
	float fraction;
	bool hit;

	bool operator<(const B2RayCastResult& rhs) const {
		return fraction < rhs.fraction;
	}
};

struct B2RayCastContext
{
	using FilterFn = std::function<float(const Handle<B2Shape>&)>;

	std::set<B2RayCastResult> results;
	FilterFn filter;
};

class B2RayCastCallback
{
public:
	/*static float FindClosestShape(b2ShapeId shapeId, b2Vec2 point, b2Vec2 normal, float fraction, void* context)
	{
		B2RayCastReturn cast{ fraction };

		if (!context)
		{
			return cast.Terminate();
		}

		auto ctx = static_cast<B2RayCastContext*>(context);

		auto handle = Handle<B2Shape>::Create(shapeId);
		if (ctx->shapes.empty())
		{
			ctx->shapes.push_back(handle);
		}
		else
		{
			assert(ctx->shapes.size() == 1);
			ctx->shapes[0] = handle;
		}

		ctx->point = ToSDLFPointScaled(point);
		ctx->normal = ToSDLFPoint(normal);
		ctx->fraction = fraction;

		return cast.Clip();
	}*/

	static float FindAllShapes(b2ShapeId shapeId, b2Vec2 point, b2Vec2 normal, float fraction, void* context)
	{
		B2RayCastReturn cast{ fraction };

		if (!context)
		{
			return cast.Terminate();
		}
		if (!b2Shape_IsValid(shapeId))
		{
			return cast.Ignore();
		}

		auto ctx = static_cast<B2RayCastContext*>(context);

		auto shapeHandle = Handle<B2Shape>::Create(shapeId);
		assert(shapeHandle.IsValid());

		float ret;
		if (ctx->filter)
		{
			ret = ctx->filter(shapeHandle);
			if (ret == cast.Ignore())
			{
				return ret;
			}
		}
		else
		{
			ret = cast.Continue();
		}

		ctx->results.insert(B2RayCastResult{
			.shapeHandle = shapeHandle,
			.point = ToSDLFPointScaled(point),
			.normal = ToSDLFPoint(normal),
			.fraction = fraction,
			.hit = true
		});

		return ret;
	}

private:
	B2RayCastCallback() = default; 
};

// TODO: implement b2 query filtering
class B2RayCaster
{
public:
	B2RayCaster() = default;

	const B2RayCastContext& GetContext() const { return context_; }

	void ClearContext()
	{
		context_.results.clear();
		context_.filter = nullptr;
	}

	const B2RayCastContext& CastRay(b2WorldId worldId, SDL_FPoint pointA, SDL_FPoint pointB, b2CastResultFcn fn)
	{
		assert(b2World_IsValid(worldId));

		auto origin = ToB2VecScaled(pointA);
		auto dest = ToB2VecScaled(pointB);

		b2QueryFilter qFilter{
			.categoryBits = 0xFFFFFFFFFFFFFFFF, // Accept all categories
			.maskBits = 0xFFFFFFFFFFFFFFFF      // Can collide with all objects
		};

		b2World_CastRay(worldId, origin, dest - origin, qFilter, fn, &context_);

		return context_;
	}

	B2RayCastResult CastRayClosest(b2WorldId worldId, SDL_FPoint pointA, SDL_FPoint pointB) const
	{
		assert(b2World_IsValid(worldId));

		auto origin = ToB2VecScaled(pointA);
		auto dest = ToB2VecScaled(pointB);

		b2QueryFilter qFilter{
			.categoryBits = 0xFFFFFFFFFFFFFFFF, // Accept all categories
			.maskBits = 0xFFFFFFFFFFFFFFFF      // Can collide with all objects
		};

		auto ret = b2World_CastRayClosest(worldId, origin, dest - origin, qFilter);

		if (ret.hit)
		{
			int i = 0;
		}

		return B2RayCastResult{
			.shapeHandle = Handle<B2Shape>::Create(ret.shapeId),
			.point = ToSDLFPointScaled(ret.point),
			.normal = ToSDLFPoint(ret.normal),
			.fraction = ret.fraction,
			.hit = ret.hit
		};
	}

private:
	B2RayCastContext context_;
};

