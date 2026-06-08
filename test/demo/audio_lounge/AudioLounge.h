#pragma once
#include "../../../ecs/Ecs.h"
#include "../../../audio/AudioBank.h"
#include "../../../atlas/NewTextureRepository.h"
#include "../../../events/EventBus2.h"

namespace test {

static constexpr float kCdSpritesXSpacing = 300.0f;
static constexpr float kCdSpriteStiffness = 120.0f;
static constexpr float kCdSpriteDamping = 22.0f;

struct CDCarousel
{
    int currentSelection = 0;
    int maxSelection = 0;

    float spacing = kCdSpritesXSpacing;

    // Animated state
    float position = 0.0f;
    float velocity = 0.0f;

    // Spring tuning
    float stiffness = kCdSpriteStiffness;
    float damping = kCdSpriteDamping;

    bool MoveLeft()
    {
		const int next = std::max(0, currentSelection - 1);
		if (next == currentSelection)
		{
			return false;
		}
		currentSelection = next;
		return true;
    }

    bool MoveRight()
    {
		const int next = std::min(maxSelection, currentSelection + 1);
		if (next == currentSelection)
		{
			return false;
		}
		currentSelection = next;
		return true;
    }

	void Update(float dt);

    void Apply(std::vector<Entity>& cdSpriteEntities, std::vector<Entity>& trackNameEntities);
};

struct CarouselRequest
{
	enum Type : uint8_t { None, MoveLeft, MoveRight };

	Type type = None;
};

class AudioLounge2
{
public:
	Result<Void> Init(const std::string& audioDir, AudioBank& audioBank, TextureRepository& repo,
					  EventBus& bus);

	void Update(float dt);

private:
	Entity& GetActiveSpriteEntity();
	//SDL_FPoint GetActiveSpriteEntityPosition() const;

	Entity controllerEntity_;
	std::vector<Entity> cdSpriteEntities_;
	std::vector<Entity> trackNameEntities_;
	std::vector<Handle<Audio>> audioHandles_;
	CDCarousel carousel_;
	CarouselRequest::Type activeRequest_ = CarouselRequest::Type::None;
	float timeInCurrentCdSpriteFrame_ = 0.0f;
};

class AudioLounge
{
public:
	struct CD
	{
		Handle<Audio> audioHandle;
		Entity cdSpriteEntity;
		Entity trackNameEntity;

		void SetPosition(SDL_FPoint pos);
		void SetVisible(bool tf);
		void SetAlpha(uint8_t alpha);
	};

	Result<Void> Init(const std::string& audioDir, AudioBank& audioBank, TextureRepository& repo);

	void Update(float dt);

private:
	enum GoNextCD { None, Left, Right };

	struct CDSpritePositions
	{
		SDL_FPoint left = { 0.0f, 0.0f };
		SDL_FPoint center = { 0.0f, 0.0f };
		SDL_FPoint right = { 0.0f, 0.0f };
	};

	static CDSpritePositions MakeCDSpritePositions();

	//struct ActiveCDState
	//{
	//	float timeInCurrentFrame = 0.0f;
	//};

	std::vector<CD> cds_;
	size_t currentCdIndex_ = 0;
	GoNextCD goNextCd_ = GoNextCD::None;
	float timeInCurrentCdSpriteFrame = 0.0f;
	CDSpritePositions cdSpritePositions_;
	bool panningCds_ = false;
};










} // test