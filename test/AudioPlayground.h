#pragma once
#include <cassert>
#include "Fixtures.h"


namespace test {

//class AudioPlaylist
//{
//public:
//	AudioPlaylist(int cellW, int cellH)
//
//private:
//};

//class AudioPlaylist
//{
//public:
//	AudioPlaylist(int w, int h) : self_(ECS::CreateEntity()), playlistDimensions_(w, h) 
//	{
//		
//	}
//
//	void SetAudioHandleMap(UnorderedDictionary<Handle<Audio>>&& map)
//	{
//		audioHandleMap_ = std::move(map);
//		for (size_t i = audioHandleMap_.size(); i < playlistEntities_.size(); i++)
//		{
//			playlistEntities_[i].Destroy();
//		}
//		
//		playlistEntities_.resize(audioHandleMap_.size());
//
//		auto relations = self_.GetRelations();
//		size_t entIdx = 0;
//		int runningHeight = 0;
//		for (const auto& [nm, _] : audioHandleMap_)
//		{
//			auto& entity = playlistEntities_[entIdx];
//			if (!entity.IsValid())
//			{
//				entity = relations.AddChild();
//			}
//
//
//
//			auto& renderable = entity.AddComponent<Renderable>();
//			renderable.renderData = TextRenderable{
//				.text = nm,
//				.dimensions = { playlistDimensions_.w, 20 }
//			};
//		}
//	}
//
//private:
//	Entity self_;
//	std::vector<Entity> playlistEntities_;
//	UnorderedDictionary<Handle<Audio>> audioHandleMap_;
//	Dimensions<int> playlistDimensions_;
//};
//
//class AudioPlayground
//{
//public:
//	AudioPlayground(EventBus2& bus);
//
//private:
//	Entity self_;
//	UnorderedDictionary<AudioDescriptor> audioDescriptors_;
//};

bool ProceedProcessInput(const Entity& entity, const events::GameControllerInput& ev)
{
	assert(entity.IsValid());

	if (!(entity.HasComponent<GameControllerState>() &&
		  entity.HasComponent<ActiveAudio>()))
	{
		return false;
	}

	auto& gc = entity.GetComponent<GameControllerState>();
	if (gc.joystickID != ev.joystickID)
	{
		return false;
	} 

	const auto& aa = entity.GetComponent<ActiveAudio>();
	if (aa.status == AudioStatus::Staged || aa.status == AudioStatus::Stopped)
	{
		return false;
	}

	return true;
}

auto MakeConnectControllerCallback(Entity& entity)
{
	return [entity](const events::GameControllerConnected& ev) mutable {
		if (!entity.HasComponent<GameControllerState>()) { return; }

		auto& gc = entity.GetComponent<GameControllerState>();
		if (gc.joystickID == GameController::kInvalidJoystickID)
		{
			gc.joystickID = ev.joystickID;
		}
	};	
}

auto MakeDisconnectControllerCallback(Entity& entity)
{
	return [entity](const events::GameControllerDisconnected& ev) mutable {
		if (!entity.HasComponent<GameControllerState>()) { return; }

		auto& gc = entity.GetComponent<GameControllerState>();
		if (gc.joystickID == ev.joystickID)
		{
			gc.joystickID = GameController::kInvalidJoystickID;
		}
	};
}

auto MakePlayPauseCommandCallback(Entity& entity)
{
	return [entity](const events::GameControllerInput& ev) mutable {
		if (!ProceedProcessInput(entity, ev)) { return; }

		const auto& activeAudio = entity.GetComponent<ActiveAudio>();
		auto& updateReq = entity.AddComponent<AudioUpdateRequest>();
		updateReq.instanceId = activeAudio.instanceId;
		updateReq.command = (activeAudio.status == AudioStatus::Playing)
			? AudioPlayCommand::Pause : AudioPlayCommand::Resume;
	};
}

auto MakeVolumeChangeCallback(Entity& entity, int amount)
{
	return [entity, amount](const events::GameControllerInput& ev) mutable {
		if (!ProceedProcessInput(entity, ev)) { return; }

		const auto& activeAudio = entity.GetComponent<ActiveAudio>();
		auto& updateReq = entity.AddComponent<AudioUpdateRequest>();
		updateReq.instanceId = activeAudio.instanceId;
		updateReq.settings.volume = activeAudio.settings.volume + amount;
	};
}

void MapAudioPlayControls(Entity& entity, EventBus2& bus)
{
	using namespace events;
	using Src = GameControllerInputSource;

	auto& tks = entity.AddComponent<SignalTokenStorage>().signalTokens;

	tks.emplace_back(bus.ConnectToInput(Src::A, 
		MakePlayPauseCommandCallback(entity)));
	tks.emplace_back(bus.ConnectToInput(Src::DPadUp, 
		MakeVolumeChangeCallback(entity, 10)));
	tks.emplace_back(bus.ConnectToInput(Src::DPadDown,
		MakeVolumeChangeCallback(entity, -10)));
}

















}