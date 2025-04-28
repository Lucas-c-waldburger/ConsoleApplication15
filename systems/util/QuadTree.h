//#pragma once
//#include <SDL.h>
//#include <array>
//#include <memory>
//#include <vector>
//#include "../../core/entity/UnorderedEntityIdSet.h"
//#include "../../core/entity/EntityColliderBounds.h"	
//
//class Entity;
//
//class QuadTree
//{
//public:
//	class Node
//	{
//	public:
//		static constexpr size_t kMaxLevel = 5;
//		static constexpr size_t kMaxEntitiesPerNode = 4;
//
//		explicit Node(AABB bounds, size_t lvl = 0) : boundingBox(bounds), level(lvl) {}
//
//		AABB boundingBox{};
//		size_t level = 0;
//		UnorderedEntityIdSet entityIds;
//		std::array<std::unique_ptr<Node>, 4> children;
//
//		void Subdivide();
//		bool Insert(const Entity_t entityId, const AABB& bounds);
//		void Query(const AABB& area, std::vector<EntityColliderBounds>& result, uint8_t flagsFilter);
//
//		bool IsLeaf() const { return !children[0]; }
//	};
//
//	explicit QuadTree(AABB bounds) : root_(bounds) {}
//
//	bool Insert(const Entity_t entityId, const AABB& bounds) { return root_.Insert(entityId, bounds); }
//
//	std::vector<EntityColliderBounds> GetIntersecting(const AABB& area, uint8_t flagsFilter = 0x00);
//
//private:
//	Node root_;
//};
//
