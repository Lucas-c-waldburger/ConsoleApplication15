#pragma once
#include <concepts>

class Entity;
struct RigidBody;
struct Collider;

void CleanupComponent(Entity&&, RigidBody&);
void CleanupComponent(Entity&&, Collider&);

template <typename T>
concept ComponentRequiresCleanup = requires(Entity&& e, T& cmp) {
	{ CleanupComponent(e, cmp) } -> std::same_as<void>;
};
