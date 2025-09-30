#pragma once
#include "System.h"
#include "../audio/Sound.h"

class Entity;

class AudioSystem : public System
{
public:
	void Update();
	void EntityDestroyed(Entity& entity); 

	void SetAudioBank(AudioBank&& bank)
	{
		audioManager_.ClearChannels();
		audioManager_.ClearStage();

		audioBank_ = std::move(bank);
	}

private:
	AudioBank audioBank_;
	AudioManager audioManager_;
};