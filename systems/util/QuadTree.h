#pragma once
#include <SDL.h>
#include <array>
#include <memory>
#include <vector>

class Entity;

struct AABB : public SDL_FRect
{
	constexpr AABB() : SDL_FRect{ 0.f, 0.f, 0.f, 0.f } {}
	constexpr AABB(float x_, float y_, float w_, float h_) :
		SDL_FRect{ x_, y_, w_, h_ } {}

	constexpr bool Contains(const AABB& other) const;
	constexpr bool Intersects(const AABB& other) const;

	static AABB CreateFromEntity(const Entity& entity);
};

class QuadTree
{
public:
	class Node
	{
	public:
		static constexpr size_t kMaxLevel = 5;
		static constexpr size_t kMaxEntitiesPerNode = 4;

		explicit Node(AABB bounds, size_t lvl = 0) : boundingBox(bounds), level(lvl) {}

		AABB boundingBox{};
		size_t level = 0;
		std::vector<Entity*> entities;
		std::array<std::unique_ptr<Node>, 4> children;

		void Subdivide();
		bool Insert(Entity* entity);
		void Query(const AABB& area, std::vector<Entity*>& result);

		bool IsLeaf() const { return !children[0]; }
	};

	explicit QuadTree(AABB bounds) : root_(bounds) {}

	bool Insert(Entity* entity) { return root_.Insert(entity); }

	std::vector<Entity*> Query(const AABB& area);

private:
	Node root_;
};


//struct EntityBounds
//{
//	Entity* entity = nullptr;
//	AABB bounds = { 0.0f, 0.0f, 0.0f, 0.0f };
//};