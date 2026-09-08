#pragma once
#include "AssetGridViewerChild.h"

#if IMGUI_ENABLED
#include "../InspectorCommon.h"

namespace ui {

class AssetPreviewViewerChild
{
public:
	using SpriteAssetGridSelection = AssetGridViewerChild::SpriteAssetGridSelection;

	struct ResourceContext
	{
		const SpriteAssetGridSelection& spriteSelection;
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

	struct SpriteSeriesAnimator
	{
		float animationSpeed = 0.2f;
		float elapsed = 0.0f;
		SpriteSeriesIndex index;
		uint8_t state = 0;

		void Reset() { *this = {}; }
	};

	void Draw(SceneFixture& fixture, ResourceContext& ctx);

	Result<Void> Init(SceneFixture& fixture);

private:
	void DrawSpriteAssetPreview(ResourceContext& ctx,
								const SpriteAtlas& spriteAtlas, float dt);

	void DrawSpriteAssetSinglePreview(const Sprite& sprite, 
									  const GuiTextureConverter& loadTargetConverter,
									  const SpriteAtlas& spriteAtlas);
	void DrawSpriteSeriesAssetsPreview(const std::vector<Sprite>& sprites,
									  const GuiTextureConverter& loadTargetConverter,
									  const GuiTextureConverter& uiTexturesConverter,
									  const SpriteAtlas& spriteAtlas, float dt);

	void DrawPlayerButtons(const GuiTextureConverter& uiTexturesConverter);

	Result<Void> LoadResources(SceneFixture& fixture);

	PlayerIcons playerIcons_;
	SpriteSeriesAnimator spriteSeriesAnimator_;
};


} // ui

#endif
