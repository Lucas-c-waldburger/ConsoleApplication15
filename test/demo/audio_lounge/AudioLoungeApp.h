#pragma once
#include "AudioLounge.h"
#include "../../Fixtures.h"


namespace test {

inline Result<Void> RunAudioLoungeApp(SceneFixture::SharedPtr& scene)
{
	auto path = FilePathUtility::GetRootPath() / kResourcesDirName / "audio/music";

	auto& lounge = scene->RegisterSystem<AudioLounge2>(Phase::Input);
	TRY(lounge.Init(path.string(), scene->GetAudioBank(), scene->GetTextureRepository(),
		scene->GetEventBus()));

	//auto& lounge = scene->RegisterSystem<AudioLounge>(Phase::Input);

	//TRY(lounge.Init(path.string(), scene->GetAudioBank(), scene->GetTextureRepository()));

	TRY(scene->RunGameLoop());

	return kVoid;
}


} // test