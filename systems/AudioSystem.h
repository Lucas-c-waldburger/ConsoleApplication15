#pragma once
#include "System.h"
#include "../audio/AudioManager.h"
#include "../audio/AudioBank.h"

class Entity;

class AudioSystem : public System
{
public:
	void Update(float dt);

	void EntityDestroyed(Entity& entity);

	void SetAudioBank(AudioBank&& bank);
	AudioBank& GetAudioBank() { return audioBank_; }
	const AudioBank& GetAudioBank() const { return audioBank_; }
	AudioBank&& SwapAudioBank(AudioBank&& newBank);

private:
	void HandleAudioUpdateRequests();
	void HandleNewAudioRequests();
	void UpdateActiveAudioComponents();

	void CleanupForNewAudioBank();

	Result<Void> ResolveUpdateRequestInstanceId(Entity& entity);

	AudioBank audioBank_;
	AudioManager audioManager_;
};