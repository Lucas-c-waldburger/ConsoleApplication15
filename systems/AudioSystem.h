#pragma once
#include "System.h"
#include "Pausable.h"
#include "Observers.h"
#include "../audio/AudioManager.h"
#include "../audio/AudioBank.h"

class Entity;

class AudioSystem : public System,
				    public Pausable<AudioSystem>,
					public EntityDestroyedObserver<AudioSystem>
{
public:
	friend class Pausable<AudioSystem>;
	friend class EntityDestroyedObserver<AudioSystem>;

	AudioSystem();

	void Update(float dt);

	void SetAudioBank(AudioBank&& bank);
	AudioBank& GetAudioBank() { return audioBank_; }
	const AudioBank& GetAudioBank() const { return audioBank_; }
	AudioBank&& SwapAudioBank(AudioBank&& newBank);

private:
	void SetPausedImpl(bool doPause);

	void HandleAudioUpdateRequests();
	void HandleNewAudioRequests();
	void CleanExpiredAudioInstances(std::vector<Entity>& entities);
	void UpdateActiveAudioEntities(std::vector<Entity>& entities);

	void CleanupForNewAudioBank();

	Result<Void> ResolveUpdateRequestInstanceId(Entity& entity);

	void OnEntityDestroyed(Entity entity);

	AudioBank audioBank_;
	AudioManager audioManager_;
};