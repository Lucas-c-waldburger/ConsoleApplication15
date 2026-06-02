#pragma once
#include "Gallery.h"
#include "../../Fixtures.h"

namespace test {

static constexpr std::string_view kGalleryTestPath = R"(D:\New folder)";

Result<Void> RunGalleryDemo(SceneFixture::SharedPtr& fixture)
{
	assert(fixture);

	auto& gallery = fixture->RegisterSystem<Gallery>(Phase::Input);
	TRY(gallery.Init(fixture->GetCamera(), fixture->GetTextureRepository(), fixture->GetRenderer(),
					 fixture->GetEventBus(), kGalleryTestPath));

	TRY(fixture->RunGameLoop());

	return kVoid;
}


} // test

