#pragma once
//#include "../../physics/B2Body.h"
//#include "../../ecs/Ecs.h"


namespace util {

//inline SDL_FRect ComputeBoundingBoxForShapes(const std::vector<ReadOnly<B2Shape>>& shapes)
//{
//	if (shapes.empty())
//	{
//		return { 0, 0, 0, 0 };
//	}
//
//	auto firstBbox = shapes[0].GetData().GetBoundingBox();
//
//	float minX = firstBbox.x;
//	float minY = firstBbox.y;
//	float maxX = firstBbox.x + firstBbox.w;
//	float maxY = firstBbox.y + firstBbox.h;
//
//	for (size_t i = 1; i < shapes.size(); ++i)
//	{
//		auto bbox = shapes[i].GetData().GetBoundingBox();
//
//		minX = std::min(minX, bbox.x);
//		minY = std::min(minY, bbox.y);
//		maxX = std::max(maxX, bbox.x + bbox.w);
//		maxY = std::max(maxY, bbox.y + bbox.h);
//	}
//
//	return { minX, minY, maxX - minX, maxY - minY };
//}
//
//inline SDL_FRect GetEntityBoundingBox(const Entity& e)
//{
//	if (e.HasComponent<RigidBody>())
//	{
//		const auto& body = e.GetComponent<RigidBody>().body.GetData();
//		if (body.IsValid())
//		{
//			auto shapes = body.GetShapes();
//			if (!shapes.empty())
//			{
//				return ComputeBoundingBoxForShapes(shapes);
//			}
//		}
//	}
//
//	if (!e.HasComponent<Transform>())
//	{
//		return { 0.0f, 0.0f, 0.0f, 0.0f };
//	}
//
//	const auto& tf = e.GetComponent<Transform>();
//
//	if (e.HasComponent<SpriteRenderableComponent>())
//	{
//		const auto& 
//	}
//}

} // util