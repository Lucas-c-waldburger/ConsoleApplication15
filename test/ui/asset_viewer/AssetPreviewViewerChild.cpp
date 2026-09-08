#include "AssetPreviewViewerChild.h"

#if IMGUI_ENABLED
#include "../../Fixtures.h"

namespace ui {

static constexpr float kPreviewTextureSize = 256.0f;
static constexpr float kPreviewSeriesAnimationIndexSliderSize = 300.0f;
static constexpr float kPreviewSeriesAnimationSpeedSliderSize = 100.0f;
static constexpr float kPlayerButtonSize = 64.0f;
static constexpr float kPlayerButtonPadding = 15.0f;
//static constexpr float kPlayerButtonLayoutWidth = 
//	(kPlayerButtonSize * 3.0f) + (kPlayerButtonPadding * 2.0f);
static constexpr float kPlayerButtonsTotalWidth = kPlayerButtonSize * 3.0f;

void AssetPreviewViewerChild::Draw(SceneFixture& fixture, ResourceContext& ctx)
{
	if (ctx.spriteSelection.HasSelection())
	{
		DrawSpriteAssetPreview(
			ctx,
			fixture.GetTextureRepository().GetSpriteAtlas(),
			fixture.GetDeltaTime());
	}
}


void AssetPreviewViewerChild::DrawSpriteAssetPreview(ResourceContext& ctx,
													 const SpriteAtlas& spriteAtlas, float dt)
{
	if (ctx.spriteSelection.SpriteSeriesSelectedAtTopLevel())
	{
		auto sprites = spriteAtlas.GetSpriteSeries(ctx.spriteSelection.spriteSeries);

		DrawSpriteSeriesAssetsPreview(sprites, 
			ctx.loadTargetConverter, 
			ctx.uiTexturesConverter,
			spriteAtlas, dt);

		//for (size_t i = 0; i < sprites.size(); ++i)
		//{
		//	DrawSpriteAssetSinglePreview(sprites[i], loadTargetConverter, spriteAtlas);

		//	if (i < sprites.size() - 1)
		//	{
		//		ImGui::SameLine();
		//	}
		//}
	}
	else if (spriteAtlas.IsSpriteValid(ctx.spriteSelection.sprite))
	{
		DrawSpriteAssetSinglePreview(ctx.spriteSelection.sprite, ctx.loadTargetConverter, spriteAtlas);
	}
}

void AssetPreviewViewerChild::DrawSpriteAssetSinglePreview(const Sprite& sprite, 
														   const GuiTextureConverter& loadTargetConverter,
														   const SpriteAtlas& spriteAtlas)
{
	auto tx = loadTargetConverter.FromSprite(sprite);
	assert(tx.textureId != 0);
	
	const float txScale = std::min(
		kPreviewTextureSize / tx.size.x,
		kPreviewTextureSize / tx.size.y
	);

	tx.size.x *= txScale;
	tx.size.y *= txScale;

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
		(ImGui::GetContentRegionAvail().x - tx.size.x) * 0.5f);
	
	GuiImage(tx);

	ImGui::Separator();

	ImGui::Dummy(ImVec2(0.0f, 12.0f));

	const auto info = spriteAtlas.GetSpriteInfo<&SpriteInfo::spriteName,
												&SpriteInfo::filepath>(sprite);
	assert(info.has_value());

	const auto& [spriteName, filepath] = *info;

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 
		(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(spriteName.c_str()).x) * 0.5f);

	ImGui::TextUnformatted(spriteName.c_str());

	ImGui::Dummy(ImVec2(0.0f, 12.0f));

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
		(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(filepath.c_str()).x) * 0.5f);

	ImGui::TextUnformatted(filepath.c_str());

	ImGui::Dummy(ImVec2(0.0f, 12.0f));

	const auto dimensionsStr = std::format("{} x {}", sprite.plot.rect.w, sprite.plot.rect.h);

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
		(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(dimensionsStr.c_str()).x) * 0.5f);

	ImGui::TextUnformatted(dimensionsStr.c_str());
}

void AssetPreviewViewerChild::DrawSpriteSeriesAssetsPreview(const std::vector<Sprite>& sprites, 
														    const GuiTextureConverter& loadTargetConverter,
														    const GuiTextureConverter& uiTexturesConverter, 
															const SpriteAtlas& spriteAtlas, float dt)
{
	if (sprites.empty())
	{
		return;
	}

	auto& animator = spriteSeriesAnimator_;

	animator.index.max = sprites.size() - 1;
	animator.index.current = std::min(animator.index.current, animator.index.max);

	if (animator.state & PlayerState::Playing)
	{
		animator.elapsed += dt;
		if (animator.elapsed >= animator.animationSpeed)
		{
			++animator.index;
			animator.elapsed = 0.0f;
		}
	}

	assert(animator.index.current < sprites.size());
	
	auto tx = loadTargetConverter.FromSprite(sprites[animator.index.current]);
	assert(tx.textureId != 0);

	const float txScale = std::min(
		kPreviewTextureSize / tx.size.x,
		kPreviewTextureSize / tx.size.y
	);

	tx.size.x *= txScale;
	tx.size.y *= txScale;

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
		(ImGui::GetContentRegionAvail().x - tx.size.x) * 0.5f);

	GuiImage(tx);

	ImGui::Separator();

	ImGui::Dummy(ImVec2(0.0f, 12.0f));

	// index slider
	float indexSliderPos = std::max(
		(ImGui::GetContentRegionAvail().x - kPreviewSeriesAnimationIndexSliderSize) * 0.5f,
		0.0f);

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + indexSliderPos);
	ImGui::SetNextItemWidth(kPreviewSeriesAnimationIndexSliderSize);

	int idxDrag = static_cast<int>(animator.index.current);
	if (ImGui::DragInt("##index", &idxDrag, 1.0f, 0, static_cast<int>(animator.index.max)))
	{
		animator.index.current = static_cast<size_t>(idxDrag);
	}

	ImGui::Dummy(ImVec2(0.0f, 12.0f));

	// player buttons
	DrawPlayerButtons(uiTexturesConverter);

	ImGui::Dummy(ImVec2(0.0f, 12.0f));

	// speed slider
	const float speedTextWidth = ImGui::CalcTextSize("speed:").x;

	const float speedSliderPos = std::max(
		(ImGui::GetContentRegionAvail().x - kPreviewSeriesAnimationSpeedSliderSize) * 0.5f,
		0.0f);

	const float speedTextStartX = speedSliderPos - ImGui::GetStyle().ItemSpacing.x - speedTextWidth;

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + speedTextStartX);

	ImGui::TextUnformatted("speed:");

	ImGui::SameLine();

	ImGui::SetNextItemWidth(kPreviewSeriesAnimationSpeedSliderSize);
	
	animator.animationSpeed = std::max(animator.animationSpeed, 0.001f);

	ImGui::DragFloat("##speed", &animator.animationSpeed, 0.001f, 0.001f);
}

void AssetPreviewViewerChild::DrawPlayerButtons(const GuiTextureConverter& uiTexturesConverter)
{
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

	auto& animator = spriteSeriesAnimator_;

	const float spacing = ImGui::GetStyle().ItemSpacing.x;

	const float totalLayoutWidth = kPlayerButtonsTotalWidth + (3.0f * spacing);

	const float centerOffset = std::max(
		(ImGui::GetContentRegionAvail().x - totalLayoutWidth) * 0.5f,
		0.0f);

	const float startX = centerOffset;

	ImGui::SetCursorPosX(startX);

	auto rewindTx = uiTexturesConverter.FromSprite(playerIcons_.rewindSprite);
	assert(rewindTx.textureId != 0);

	if (GuiImageButton("rewindBtn", rewindTx))
	{
		--animator.index;
	}

	ImGui::SameLine(0.0f, spacing);

	const auto& playPauseSprite = (animator.state & PlayerState::Playing)
		? playerIcons_.pauseSprite
		: playerIcons_.playSprite;

	auto playPauseTx = uiTexturesConverter.FromSprite(playPauseSprite);
	assert(playPauseTx.textureId != 0);

	if (GuiImageButton("playPauseBtn", playPauseTx))
	{
		animator.state ^= PlayerState::Playing;
	}

	ImGui::SameLine(0.0f, spacing);

	auto fastForwardTx = uiTexturesConverter.FromSprite(playerIcons_.fastForwardSprite);
	assert(fastForwardTx.textureId != 0);

	if (GuiImageButton("fastForwardBtn", fastForwardTx))
	{
		++animator.index;
	}

	ImGui::PopStyleColor();
}

Result<Void> AssetPreviewViewerChild::LoadResources(SceneFixture& fixture)
{
	auto& auxRepo = fixture.GetAuxTextureRepository();
	if (!auxRepo)
	{
		return MAKE_ERROR("Auxiliary texture repository was null");
	}

	TRY(ResourcePath::Sprite("ui/editor/asset_player_play_icon_large.png"),
		playIconPath);
	TRY(ResourcePath::Sprite("ui/editor/asset_player_pause_icon_large.png"),
		pauseIconPath);
	TRY(ResourcePath::Sprite("ui/editor/asset_player_rewind_icon_large.png"),
		rewindIconPath);
	TRY(ResourcePath::Sprite("ui/editor/asset_player_fast_forward_icon_large.png"),
		fastForwardIconPath);

	auto& atlas = auxRepo->GetSpriteAtlas();

	TRY_ASSIGN(playerIcons_.playSprite, atlas.LoadSprite(
		fixture.GetRenderer(), { .filepath = std::move(playIconPath) }));
	TRY_ASSIGN(playerIcons_.pauseSprite, atlas.LoadSprite(
		fixture.GetRenderer(), { .filepath = std::move(pauseIconPath) }));
	TRY_ASSIGN(playerIcons_.rewindSprite, atlas.LoadSprite(
		fixture.GetRenderer(), { .filepath = std::move(rewindIconPath) }));
	TRY_ASSIGN(playerIcons_.fastForwardSprite, atlas.LoadSprite(
		fixture.GetRenderer(), { .filepath = std::move(fastForwardIconPath) }));

	return kVoid;
}


Result<Void> AssetPreviewViewerChild::Init(SceneFixture& fixture)
{
	TRY(LoadResources(fixture));

	return kVoid;
}


} // ui

#endif