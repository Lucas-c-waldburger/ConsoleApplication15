#pragma once
#include "System.h"
#include "../audio/AudioManager.h"

class Entity;

class AudioSystem : public System
{
public:
	void Update();

	void EntityDestroyed(Entity& entity);

	void SetAudioBank(AudioBank&& bank);

private:
	void HandleAudioUpdateRequests();
	void HandleNewAudioRequests();
	void UpdateActiveAudioComponents();

	Result<Void> CheckActiveAudioAndUpdateRequestConsistency(const Entity& entity);

	AudioBank audioBank_;
	AudioManager audioManager_;
};