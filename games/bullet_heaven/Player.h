#pragma once
#include "Components.h"
#include "../../test/Fixtures.h"

namespace game {

class Player
{
public:
	static Player Get();

	Result<Void> Init(std::shared_ptr<SceneFixture>& fixture);

private:
	Player() = default;

	static inline Entity entity_{};
};

} // game