#pragma once
#include "System.h"
#include "Pausable.h"
#include "Observers.h"
#include "../audio/AudioManager2.h"

class AudioSystem2 : public System,
				     public Pausable<AudioSystem2>,
				     public EntityDestroyedObserver<AudioSystem2>
{
public:
	friend class Pausable<AudioSystem2>;
	friend class EntityDestroyedObserver<AudioSystem2>;

	void Update(float dt, AudioBank& audioBank);

private:
	void HandleAudioUpdateRequests(AudioBank& audioBank);

	void PreprocessNewAudioRequests(std::vector<Entity>& newAudioReqEs, AudioBank& audioBank);

	void StageNewAudioRequests(std::vector<Entity>& newAudioReqEs, AudioBank& audioBank, float dt);

	void UpdateActiveAudio();

	void SetPausedImpl(bool doPause);

	void OnEntityDestroyed(Entity entity);

	AudioManager2 audioManager_;
};