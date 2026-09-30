#pragma once
#include "AssetGridViewerChild.h"

#if IMGUI_ENABLED
#include "../InspectorCommon.h"
#include "MusicVisualizer.h"
#include "../lua_text_editor/LuaTextEditor.h"

class ScriptSystem;

namespace ui {

struct AudioPlayerContext;

class AssetPreviewViewerChild
{
public:
	using SpriteAssetGridSelection = AssetGridViewerChild::SpriteAssetGridSelection;
	using AudioAssetGridSelection = AssetGridViewerChild::AudioAssetGridSelection;
	using FontAssetGridSelection = AssetGridViewerChild::FontAssetGridSelection;
	using ScriptAssetGridSelection = AssetGridViewerChild::ScriptAssetGridSelection;

	struct ResourceContext
	{
		const SpriteAssetGridSelection& spriteSelection;
		const AudioAssetGridSelection& audioSelection;
		const FontAssetGridSelection& fontSelection;
		const ScriptAssetGridSelection& scriptSelection;
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

		void Reset() { *this = SpriteSeriesPlayer{}; }
	};

	struct AudioPlayer
	{
		Entity_t entityId = kInvalidEntity;
		uint8_t state = 0;
		MusicVisualizer musicVisualizer;

		std::optional<AudioPlayerContext> MakePlayerContext(const AudioBank& audioBank);

		void Update(const Handle<Audio>& handle, const AudioBank& audioBank);
		void Reset();
	};

	struct FontWriterDisplay
	{
		std::string text = "The quick brown fox jumps over the lazy dog";
		ImVec4 color{ 1.0f, 0.0f, 0.0f, 1.0f };
		uint8_t state = 0;

		void Reset() { *this = FontWriterDisplay{}; }
	};

	struct ScriptEditorDisplay
	{
		LuaTextEditor textEditor;
		ScriptTable::TableId sourceTableId = ScriptTable::kInvalidTableId;

		void Reset() 
		{ 
			textEditor.Clear();
			textEditor.SetReadOnly(true); 
		}
	};

	const SpriteSeriesPlayer& GetSpriteSeriesAnimator() const { return spriteSeriesAnimator_; }
	const AudioPlayer& GetAudioPlayer() const { return audioPlayer_; }
	const FontWriterDisplay& GetFontWriterDisplay() const { return fontWriterDisplay_; }
	const ScriptEditorDisplay& GetScriptEditorDisplay() const { return scriptEditorDisplay_; }

	void Draw(SceneFixture& fixture, ResourceContext& ctx, 
			  const DataRecord<AssetItem::Type>& assetGridTabType);

	Result<Void> Init(SceneFixture& fixture);

	void Reset();

	void TearDown();

private:
	void DrawSpriteAssetPreview(ResourceContext& ctx,
								SpriteAtlas& spriteAtlas, float dt);
	void DrawAudioAssetPreview(ResourceContext& ctx,
							   const AudioBank& audioBank);
	void DrawFontAssetPreview(ResourceContext& ctx);

	void DrawScriptAssetPreview(ResourceContext& ctx, ScriptSystem& scriptSys);

	void DrawSpriteAssetSinglePreview(const Sprite& sprite, 
									  const GuiTextureConverter& loadTargetConverter,
									  const SpriteAtlas& spriteAtlas);
	void DrawSpriteSeriesAssetsPreview(std::vector<Sprite>& sprites,
									   std::string_view seriesName,
									   const GuiTextureConverter& loadTargetConverter,
									   const GuiTextureConverter& uiTexturesConverter,
									   SpriteAtlas& spriteAtlas, float dt);

	void DrawSpriteSeriesPlayerButtons(const GuiTextureConverter& uiTexturesConverter);
	void DrawAudioPlayerButtons(const GuiTextureConverter& uiTexturesConverter, 
								std::optional<AudioPlayerContext>& audioPlayerCtx);

	void DrawSpriteSeriesReorderLayout(std::vector<Sprite>& sprites,
									   std::string_view seriesName,
									   const GuiTextureConverter& loadTargetConverter,
									   SpriteAtlas& spriteAtlas);

	Result<Void> LoadResources(SceneFixture& fixture);

	PlayerIcons playerIcons_;
	SpriteSeriesPlayer spriteSeriesAnimator_;
	AudioPlayer audioPlayer_;
	FontWriterDisplay fontWriterDisplay_{};
	ScriptEditorDisplay scriptEditorDisplay_{};
	AssetItem::Type openAssetTab_ = AssetItem::Type::Unknown;
};


} // ui

#endif
