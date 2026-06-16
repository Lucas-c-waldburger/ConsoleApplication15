#include <cassert>
#include "../CatchUtils.h"
#include "../../Fixtures.h"
#include "../../../ecs/Ecs.h"
#include "../../../file/FilePathUtility.h"

namespace {

constexpr std::string_view kGreenpathName = "greenpath";
constexpr std::string_view kFutureBeatName = "futuristic_beat";
constexpr std::string_view kBellHitName = "bell_hit";
constexpr std::string_view kBossStunName = "boss_stun";
constexpr std::string_view kChestOpenName = "chest_open";
constexpr std::string_view kCrowdGaspName = "crowd_gasp";
constexpr std::string_view kEnemyDamageName = "enemy_damage";
constexpr std::string_view kExplosion1Name = "explosion_1";
constexpr std::string_view kFocusHealthHealName = "focus_health_heal";
constexpr std::string_view kGateSlamName = "gate_slam";

constexpr std::string_view kHeroDashName = "hero_dash";
constexpr std::string_view kHeroJumpName = "hero_jump";
constexpr std::string_view kHeroParryName = "hero_parry";
constexpr std::string_view kHeroRunFootstepsStoneName = "hero_run_footsteps_stone";
constexpr std::string_view kSecretAreaDiscoveredName = "secret_area_discovered";
constexpr std::string_view kSword1Name = "sword_1";
constexpr std::string_view kSword2Name = "sword_2";
constexpr std::string_view kSword3Name = "sword_3";
constexpr std::string_view kSword4Name = "sword_4";
constexpr std::string_view kSword5Name = "sword_5";

constexpr int kBellHitDurationMs = 4000;
constexpr int kGateSlamDurationMs = 2000;

Result<std::vector<AudioDescriptor>> MakeAudioDescriptors()
{
	using AD = AudioDescriptor;

	TRY(ResourcePath::Music("greenpath.ogg"), gpFile);
	TRY(ResourcePath::Music("futuristic_beat.mp3"), fbFile);

	TRY(ResourcePath::Sound("bell_hit.flac"), bhFile);
	TRY(ResourcePath::Sound("boss_stun.wav"), bsFile);
	TRY(ResourcePath::Sound("chest_open.wav"), coFile);
	TRY(ResourcePath::Sound("crowd_gasp.wav"), cgFile);
	TRY(ResourcePath::Sound("enemy_damage.wav"), edFile);
	TRY(ResourcePath::Sound("explosion_1.wav"), exFile);
	TRY(ResourcePath::Sound("focus_health_heal.wav"), fhhFile);
	TRY(ResourcePath::Sound("gate_slam.wav"), gsFile);
	TRY(ResourcePath::Sound("hero_dash.wav"), hdFile);
	TRY(ResourcePath::Sound("hero_jump.wav"), hjFile);
	TRY(ResourcePath::Sound("hero_parry.wav"), hpFile);
	TRY(ResourcePath::Sound("hero_run_footsteps_stone.wav"), hrfsFile);
	TRY(ResourcePath::Sound("secret_area_discovered.wav"), sadFile);
	TRY(ResourcePath::Sound("sword_1.wav"), s1File);
	TRY(ResourcePath::Sound("sword_2.wav"), s2File);
	TRY(ResourcePath::Sound("sword_3.wav"), s3File);
	TRY(ResourcePath::Sound("sword_4.wav"), s4File);
	TRY(ResourcePath::Sound("sword_5.wav"), s5File);

	return std::vector<AD>{
	AD{ AudioType::Music, std::string{kGreenpathName}, std::move(gpFile) },
	AD{ AudioType::Music, std::string{kFutureBeatName}, std::move(fbFile) },

	AD{ AudioType::Sound, std::string{kBellHitName}, std::move(bhFile) },
	AD{ AudioType::Sound, std::string{kBossStunName}, std::move(bsFile) },
	AD{ AudioType::Sound, std::string{kCrowdGaspName}, std::move(cgFile) },
	AD{ AudioType::Sound, std::string{kChestOpenName}, std::move(coFile) },
	AD{ AudioType::Sound, std::string{kEnemyDamageName}, std::move(edFile) },
	AD{ AudioType::Sound, std::string{kExplosion1Name}, std::move(exFile) },
	AD{ AudioType::Sound, std::string{kFocusHealthHealName}, std::move(fhhFile) },
	AD{ AudioType::Sound, std::string{kGateSlamName}, std::move(gsFile) },
	AD{ AudioType::Sound, std::string{kHeroDashName}, std::move(hdFile) },
	AD{ AudioType::Sound, std::string{kHeroJumpName}, std::move(hjFile) },
	AD{ AudioType::Sound, std::string{kHeroParryName}, std::move(hpFile) },
	AD{ AudioType::Sound, std::string{kHeroRunFootstepsStoneName}, std::move(hrfsFile) },
	AD{ AudioType::Sound, std::string{kSecretAreaDiscoveredName}, std::move(sadFile) },
	AD{ AudioType::Sound, std::string{kSword1Name}, std::move(s1File) },
	AD{ AudioType::Sound, std::string{kSword2Name}, std::move(s2File) },
	AD{ AudioType::Sound, std::string{kSword3Name}, std::move(s3File) },
	AD{ AudioType::Sound, std::string{kSword4Name}, std::move(s4File) },
	AD{ AudioType::Sound, std::string{kSword5Name}, std::move(s5File) }
	};
};

Result<UnorderedDictionary<Handle<Audio>>>
LoadTestAudioDescriptors(SceneFixture& scene, 
					 std::vector<AudioDescriptor>&& descriptors)
{
	UnorderedDictionary<Handle<Audio>> audioHandleMap;
	audioHandleMap.reserve(descriptors.size());

	AudioBank bank{};

	for (auto&& descriptor : descriptors)
	{
		if (audioHandleMap.contains(descriptor.name))
		{
			return MAKE_ERROR_FMT("Duplicate descriptor name: '{}'", 
				descriptor.name);
		}

		auto [it, inserted] = audioHandleMap.try_emplace(descriptor.name, 
													     Handle<Audio>{});
		assert(inserted);

		TRY_ASSIGN(it->second, bank.LoadAudio(std::move(descriptor)));

		assert(it->second.IsValid());
	}

	scene.GetSystem<AudioSystem>().SetAudioBank(std::move(bank));

	return audioHandleMap;
}

const ActiveAudio& GetActiveAudio(Entity& ent)
{
	REQUIRE(ent.IsValid());
	REQUIRE(ent.HasComponent<ActiveAudio>());

	const auto& activeAudio = ent.GetComponent<ActiveAudio>();
	REQUIRE(activeAudio.audioHandle.IsValid());
	REQUIRE(activeAudio.instanceId.IsValid());

	return activeAudio;
}

} // unnamed

TEST_CASE("Audio System Tests", "[audio][system]")
{
	UnorderedDictionary<Handle<Audio>> audioHandleMap;

	auto sceneResult = SceneFixture::GetInstance();
	REQUIRE(sceneResult.Success());

	auto& scene = sceneResult.GetValue();
	REQUIRE(scene);

	auto descriptorsResult = MakeAudioDescriptors();
	REQUIRE(descriptorsResult.Success());

	auto loadResult = LoadTestAudioDescriptors(*scene,
		std::move(descriptorsResult.GetValue()));
	REQUIRE(loadResult.Success()); 

	audioHandleMap = std::move(loadResult.GetValue());

	SECTION("New Audio Request")
	{
		auto entity = ECS::CreateEntity();
		REQUIRE(entity.IsValid());

		auto& newAudioReq = entity.AddComponent<NewAudioRequest>();
		newAudioReq.audioHandle = audioHandleMap[kGreenpathName];

		auto run1 = scene->StepGameLoop(1);
		REQUIRE(run1.Success());

		REQUIRE(!entity.HasComponent<NewAudioRequest>());
		REQUIRE(entity.HasComponent<ActiveAudio>());

		const auto& activeAudio = entity.GetComponent<ActiveAudio>();
		CHECK(activeAudio.audioHandle == audioHandleMap[kGreenpathName]);
		CHECK(activeAudio.instanceId.IsValid());
		CHECK(activeAudio.onChannel == AudioManager::kMusicChannelIndex);
		CHECK(activeAudio.status == AudioStatus::Playing);
	}

	SECTION("Audio Update Requests")
	{
		auto entity = ECS::CreateEntity();
		REQUIRE(entity.IsValid());

		auto& newAudioReq = entity.AddComponent<NewAudioRequest>();
		newAudioReq.audioHandle = audioHandleMap[kGreenpathName];

		REQUIRE(scene->StepGameLoop(1).Success());

		// pause
		const auto& activeAudio1 = GetActiveAudio(entity);
		CHECK(activeAudio1.status == AudioStatus::Playing);

		auto& updateReq1 = entity.AddComponent<AudioUpdateRequest>();
		updateReq1.instanceId = activeAudio1.instanceId;
		updateReq1.command = AudioPlayCommand::Pause;
		
		REQUIRE(scene->StepGameLoop(1).Success());

		// resume
		CHECK(!entity.HasComponent<AudioUpdateRequest>());

		const auto& activeAudio2 = GetActiveAudio(entity);
		CHECK(activeAudio2.status == AudioStatus::Paused);

		auto& updateReq2 = entity.AddComponent<AudioUpdateRequest>();
		updateReq2.instanceId = activeAudio2.instanceId;
		updateReq2.command = AudioPlayCommand::Resume;
		
		REQUIRE(scene->StepGameLoop(1).Success());

		// stop with fade out
		CHECK(!entity.HasComponent<AudioUpdateRequest>());

		const auto& activeAudio3 = GetActiveAudio(entity);
		CHECK(activeAudio3.status == AudioStatus::Playing);

		auto& updateReq3 = entity.AddComponent<AudioUpdateRequest>();
		updateReq3.instanceId = activeAudio3.instanceId;
		updateReq3.command = AudioPlayCommand::Stop;
		updateReq3.settings.fadeMs = AudioFadeMs{ .out = 1000 };
		
		REQUIRE(scene->StepGameLoop(1).Success());

		// stopping 
		CHECK(!entity.HasComponent<AudioUpdateRequest>());

		const auto& activeAudio4 = GetActiveAudio(entity);
		CHECK(activeAudio4.status == AudioStatus::Stopping);	

		REQUIRE(scene->RunGameLoopMs(1500).Success());

		// faded fully, no active audio
		CHECK(!entity.HasComponent<ActiveAudio>());		
	}

	SECTION("Full Sound Stage")
	{
		std::array<Entity, 8> ents{};
		std::generate(ents.begin(), ents.end(), ECS::CreateEntity);
	
		ents[0].AddComponent(NewAudioRequest{.audioHandle = audioHandleMap[kGateSlamName]});
		ents[1].AddComponent(NewAudioRequest{.audioHandle = audioHandleMap[kCrowdGaspName]});
		ents[2].AddComponent(NewAudioRequest{.audioHandle = audioHandleMap[kEnemyDamageName]});
		ents[3].AddComponent(NewAudioRequest{.audioHandle = audioHandleMap[kHeroDashName]});
		ents[4].AddComponent(NewAudioRequest{.audioHandle = audioHandleMap[kExplosion1Name]});
		ents[5].AddComponent(NewAudioRequest{.audioHandle = audioHandleMap[kBossStunName]});
		ents[6].AddComponent(NewAudioRequest{.audioHandle = audioHandleMap[kHeroParryName]});
		ents[7].AddComponent(NewAudioRequest{.audioHandle = audioHandleMap[kHeroJumpName]});

		REQUIRE(scene->StepGameLoop(1).Success());

		for (size_t i = 0; i < ents.size(); i++)
		{
			REQUIRE(ents[i].HasComponent<ActiveAudio>());
			const auto& activeAudio = ents[i].GetComponent<ActiveAudio>();

			CHECK(activeAudio.status == AudioStatus::Playing);
			CHECK(activeAudio.instanceId.IsValid());
			CHECK(activeAudio.onChannel == i);
		}

		auto newEnt = ECS::CreateEntity();
		newEnt.AddComponent(NewAudioRequest{.audioHandle = audioHandleMap[kBellHitName]});

		REQUIRE(scene->StepGameLoop(1).Success());

		// all original ones still playing
		for (size_t i = 0; i < ents.size(); i++)
		{
			REQUIRE(ents[i].HasComponent<ActiveAudio>());
			const auto& activeAudio = ents[i].GetComponent<ActiveAudio>();

			CHECK(activeAudio.status == AudioStatus::Playing);
			CHECK(activeAudio.instanceId.IsValid());
			CHECK(activeAudio.onChannel == i);
		}

		// 9th sound staged
		REQUIRE(newEnt.HasComponent<ActiveAudio>());
		auto& newEntActiveAudio1 = newEnt.GetComponent<ActiveAudio>();

		CHECK(newEntActiveAudio1.status == AudioStatus::Staged);
		CHECK(newEntActiveAudio1.instanceId.IsValid());
		CHECK(newEntActiveAudio1.onChannel == 0);		

		REQUIRE(scene->RunGameLoopMs(kGateSlamDurationMs + 1000).Success());

		// audio in sound slot 0 replaced with new one
		CHECK(!ents[0].HasComponent<ActiveAudio>());
		REQUIRE(newEnt.HasComponent<ActiveAudio>());
		auto& newEntActiveAudio2 = newEnt.GetComponent<ActiveAudio>();

		CHECK(newEntActiveAudio2.status == AudioStatus::Playing);
		CHECK(newEntActiveAudio2.instanceId.IsValid());
		CHECK(newEntActiveAudio2.onChannel == 0);		
	}
}

TEST_CASE("Track Position updates correctly", "[audio]")
{
	auto sceneResult = SceneFixture::GetInstance();
	REQUIRE(sceneResult.Success());

	auto& scene = sceneResult.GetValue();
	REQUIRE(scene);

	REQUIRE(scene->IsSystemRegistered<AudioSystem>());
	auto& bank = scene->GetAudioBank();

	auto e = ECS::CreateEntity();
	REQUIRE(e.IsValid());

	SECTION("Music")
	{
		auto musPathResult = ResourcePath::Music("greenpath.ogg");
		REQUIRE(musPathResult.Success());

		auto musLoadResult = bank.LoadAudio({ 
			.audioType = AudioType::Music,
			.filepath = std::move(musPathResult).GetValue() 
		});

		REQUIRE(musLoadResult.Success());
		REQUIRE(musLoadResult.GetValue().IsValid());

		auto& newAudioReq = e.AddComponent(NewAudioRequest{ .audioHandle = musLoadResult.GetValue() });
		CHECK(EqualsWithTolerance(newAudioReq.settings.trackPosition, 0.0f));

		scene->StepGameLoop(1);

		CHECK_FALSE(e.HasComponent<NewAudioRequest>());
		REQUIRE(e.HasComponent<ActiveAudio>());

		float lastTrackPos = 0.0f;
		int readCount = 0;
		for (size_t i = 0; i < 10; ++i)
		{
			scene->StepGameLoop(8);
			
			if (!e.HasComponent<ActiveAudio>())
			{
				break;
			}
			const auto& activeAudio = e.GetComponent<ActiveAudio>();

			CHECK(activeAudio.status == AudioStatus::Playing);
			CHECK(activeAudio.settings.trackPosition > lastTrackPos);
			lastTrackPos = activeAudio.settings.trackPosition;

			++readCount;
		}

		CHECK(readCount >= 2);

		{
		REQUIRE(e.HasComponent<ActiveAudio>());
		const auto& activeAudio = e.GetComponent<ActiveAudio>();
		REQUIRE(activeAudio.instanceId.IsValid());

		auto& audioUpdateReq = e.AddComponent(AudioUpdateRequest{
			.instanceId = activeAudio.instanceId,
			.command = AudioPlayCommand::Stop,
			.settings = { .fadeMs = AudioFadeMs{.out = 1000 } }
		});
		
		audioUpdateReq.command = AudioPlayCommand::Stop;
		audioUpdateReq.settings.fadeMs = AudioFadeMs{ .out = 1000 };
		}

		readCount = 0;
		while (e.HasComponent<ActiveAudio>())
		{
			scene->StepGameLoop(8);

			if (!e.HasComponent<ActiveAudio>())
			{
				break;
			}

			const auto& activeAudio = e.GetComponent<ActiveAudio>();
			REQUIRE(activeAudio.instanceId.IsValid());

			CHECK(activeAudio.status == AudioStatus::Stopping);
			CHECK(activeAudio.settings.trackPosition > lastTrackPos);
			lastTrackPos = activeAudio.settings.trackPosition;

			++readCount;
		} 

		CHECK(readCount >= 2);
	}

	//SECTION("Sound")
	//{
	//	auto soundPathResult = ResourcePath::Sound("chest_open.wav");
	//	REQUIRE(soundPathResult.Success());

	//	auto soundLoadResult = bank.LoadAudio({ 
	//	.audioType = AudioType::Sound
	//	.filepath = std::move(soundPathResult).GetValue() 
	// });
	//	REQUIRE(soundLoadResult.Success());
	//	REQUIRE(soundLoadResult.GetValue().IsValid());
	//}
}