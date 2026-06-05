#pragma once
#include "System.h"
#include "../audio/AudioManager.h"
#include "../audio/AudioBank2.h"

class Entity;

class AudioSystem : public System
{
public:
	void Update();

	void EntityDestroyed(Entity& entity);

	void SetAudioBank(AudioBank&& bank);
	void SetAudioBank(AudioBank2&& bank);
	AudioBank2& GetAudioBank() { return audioBank2_; }
	const AudioBank2& GetAudioBank() const { return audioBank2_; }
	AudioBank2&& SwapAudioBank(AudioBank2&& newBank);

private:
	void HandleAudioUpdateRequests();
	void HandleNewAudioRequests();
	void UpdateActiveAudioComponents();

	void CleanupForNewAudioBank();

	Result<Void> CheckActiveAudioAndUpdateRequestConsistency(const Entity& entity);

	AudioBank audioBank_;
	AudioBank2 audioBank2_;
	AudioManager audioManager_;
};