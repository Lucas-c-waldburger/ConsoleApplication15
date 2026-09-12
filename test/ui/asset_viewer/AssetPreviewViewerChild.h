#pragma once
#include "AssetGridViewerChild.h"

#if IMGUI_ENABLED
#include "../InspectorCommon.h"
#include "MusicVisualizer.h"

namespace ui {

class AssetPreviewViewerChild
{
public:
	using SpriteAssetGridSelection = AssetGridViewerChild::SpriteAssetGridSelection;
	using AudioAssetGridSelection = AssetGridViewerChild::AudioAssetGridSelection;

	struct ResourceContext
	{
		const SpriteAssetGridSelection& spriteSelection;
		const AudioAssetGridSelection& audioSelection;
		const AssetViewerIcons& icons;
		const GuiTextureConverter& loadTargetConverter;
		const GuiTextureConverter& uiTexturesConverter;
	};

	struct PlayerIcons
	{
		Sprite playSprite;
		Sprite pauseSprite;
		Sprite rewindSprite;
		Sprite fastForwardSprite;
	};

	enum PlayerState : uint8_t
	{
		Playing = 1 << 0,
		Paused = 1 << 1,
		RewindClicked = 1 << 2,
		FastForwardClicked = 1 << 3
	};

	struct SpriteSeriesPlayer
	{
		float animationSpeed = 0.2f;
		float elapsed = 0.0f;
		SpriteSeriesIndex index;
		uint8_t state = 0;

		void Reset() { *this = {}; }
	};

	struct AudioPlayer
	{
		Entity_t entityId = kInvalidEntity;
		uint8_t state = 0;
		MusicVisualizer musicVisualizer;

		void Update(const Handle<Audio>& handle, const AudioBank& audioBank);
		void Reset();
		int GetVolume() const;
		void SetVolume(int vol);
	};

	const SpriteSeriesPlayer& GetSpriteSeriesAnimator() const { return spriteSeriesAnimator_; }
	const AudioPlayer& GetAudioPlayer() const { return audioPlayer_; }

	void Draw(SceneFixture& fixture, ResourceContext& ctx);

	Result<Void> Init(SceneFixture& fixture);

	void TearDown();

private:
	void DrawSpriteAssetPreview(ResourceContext& ctx,
								const SpriteAtlas& spriteAtlas, float dt);
	void DrawAudioAssetPreview(ResourceContext& ctx,
							   const AudioBank& audioBank);

	void DrawSpriteAssetSinglePreview(const Sprite& sprite, 
									  const GuiTextureConverter& loadTargetConverter,
									  const SpriteAtlas& spriteAtlas);
	void DrawSpriteSeriesAssetsPreview(const std::vector<Sprite>& sprites,
									  const GuiTextureConverter& loadTargetConverter,
									  const GuiTextureConverter& uiTexturesConverter,
									  const SpriteAtlas& spriteAtlas, float dt);

	void DrawSpriteSeriesPlayerButtons(const GuiTextureConverter& uiTexturesConverter);
	void DrawAudioPlayerButtons(const GuiTextureConverter& uiTexturesConverter, const AudioBank& audioBank);

	Result<Void> LoadResources(SceneFixture& fixture);

	PlayerIcons playerIcons_;
	SpriteSeriesPlayer spriteSeriesAnimator_;
	AudioPlayer audioPlayer_;
};


} // ui

#endif
