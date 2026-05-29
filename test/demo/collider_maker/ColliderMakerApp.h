#pragma once
#include "ColliderMaker2.h"
#include "../../Fixtures.h"

namespace test {

Result<Void> RunColliderMaker(SceneFixture::SharedPtr& fixture)
{
#if IMGUI_ENABLED

	TRY(ColliderMaker2::Init(fixture));

#endif

	return fixture->RunGameLoop();
}


} // test
