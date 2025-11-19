#include "Workspace.h"
#include "../../file/FilePathUtility.h"
#include "../../atlas/NewTextureRepository.h"

namespace ui {


Result<Workspace> Workspace::Create(SDL_Renderer* renderer, 
									TextureRepository& repo, EventBus2& bus)
{
	Workspace ws{};
	ws.textureRepo_ = &repo;
	ws.eventBus_ = &bus;

	for (auto [color, spritesFilenames] : Button::kSpritePathColorMap)
	{
		const auto& [upName, downName] = spritesFilenames;

		if (upName.empty() || downName.empty())
		{
			continue;
		}

		TRY(Button::LoadButtonSprites(renderer, repo, upName, downName), sprites);

		ws.buttonData_.spritesByColor[color] = std::move(sprites);
	}

	// load font
	TRY(ResourcePath::Font(Button::kButtonFontFilename), fontFilepath);

	TRY(GlyphAtlas::Create(renderer, {
		.filepath = fontFilepath, .fontSize = 24 }), glyphAtlas);

	ws.buttonData_.writer = glyphAtlas.GetTextWriter();

	TRY(repo.AttachAtlas(std::move(glyphAtlas)));

	return ws;
}

Result<Handle<Button>> Workspace::PlaceButton(Button::Params&& params)
{
	const auto& [upSprite, downSprite] = buttonData_.spritesByColor[params.color];
	if (!(upSprite.sourceAtlas.IsValid() && downSprite.sourceAtlas.IsValid()))
	{
		return MAKE_ERROR("Button sprites not valid for requested button color");
	}

	auto handle = Handle<Button>::Create();

	auto& newButton = buttonData_.buttons[handle];

	newButton.Init(
		handle,
		*eventBus_,
		buttonData_.spritesByColor[params.color],
		buttonData_.writer,
		params.position,
		std::move(params.onClick),
		params.text,
		params.scale
	);

	assert(newButton.IsValid());

	//buttonData_.buttons.try_emplace(handle, Button{
	//	handle,
	//	*eventBus_,
	//	buttonData_.spritesByColor[params.color],
	//	buttonData_.writer,
	//	params.position,
	//	std::move(params.onClick),
	//	params.text,
	//	params.scale
	//});

	return handle;
}


} // ui