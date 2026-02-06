#pragma once
#include <concepts>
#include "../ecs/EntityAccess.h"

class Entity;

//class ISystem : public EntityFullAccessPrivelage
//{
//public:
//	virtual ~ISystem() = default;
//	virtual void Update(float delta) = 0;
//};

struct ISystem {};

template <typename T>
concept ImplementsSystemUpdate = requires (T& t, float dt) {
	{ t.Update(dt) } -> std::same_as<void>;
};

template <ImplementsSystemUpdate T>
struct SystemWrapper final : public ISystem 
{ 
	template <typename...Args> requires std::constructible_from<T, Args...>
	explicit SystemWrapper(Args&&...args) : value(std::forward<Args>(args)...) {}

	T value; 
};

class System : public EntityFullAccessPrivelage
{};
