#include "Button.h"
#include "../../events/EventBus2.h"
#include "../../file/FilePathUtility.h"
#include "../../atlas/NewTextureRepository.h"

namespace ui {

auto Button::GetButtonPressCallback()
{
	return [this](const events::MouseInput& ev) {
		if (ev.input.state != InputState::Pressed ||
			ev.input.state != InputState::Released)
		{
			return;
		}

		SDL_FRect bounds = GetButtonBoundingBox();
		SDL_FPoint mousePos = ev.input.value.cursor.position.absolute;
		bool colliding = SDL_PointInFRect(&mousePos, &bounds);

		auto& sprite = self_.GetComponent<SpriteRenderableComponent>().sprite;

		if (ev.input.state == InputState::Pressed && colliding)
		{
			assert(!wasPressed_);
			wasPressed_ = true;
			sprite = sprites_.down;

			if (txt_.IsValid()) // scoot text down to look like depressed
			{
				AdjustTextYPos(3.0f);
			}
		}
		else if (ev.input.state == InputState::Released)
		{
			if (wasPressed_ && colliding && onClick_)
			{
				onClick_();
			}

			wasPressed_ = false;
			sprite = sprites_.up;

			if (txt_.IsValid()) // return text to normal position
			{
				AdjustTextYPos(-3.0f);
			}
		}
	};
}

Button::Button(const Handle<Button>& handle, EventBus2& bus, 
	const ButtonSprites& sprites, const GlyphTextWriter& writer, SDL_FPoint pos,
	Callback&& onClick, std::string_view text, SDL_FPoint scale) :
	self_(ECS::CreateEntity()), onClick_(std::move(onClick)),
	sprites_(sprites), handle_(handle)
{
	auto& tf = self_.AddComponent(Transform{ .position = pos, .scale = scale });
	auto& r = self_.AddComponent(SpriteRenderableComponent{
		.sprite = sprites.up
	});

	auto& tks = self_.AddComponent(SignalTokenStorage{});
	tks.signalTokens.push_back(bus.ConnectToInput(
		MouseInputSource::LeftButton, GetButtonPressCallback()));

	if (!text.empty())
	{
		txt_ = ECS::CreateEntity();
		auto& txTf = txt_.AddComponent<Transform>();
		txTf = tf;

		int boundsW = r.sprite.plot.rect.w * tf.scale.x * 0.8f;
		int boundsH = r.sprite.plot.rect.h * tf.scale.y * 0.8f;

		auto& txR = txt_.AddComponent(TextRenderableComponent{
			.writer = writer,
			.formatting = {
				.bounds = { boundsW, boundsH },
				.align = TextAlign::Center,
				.scaleToBounds = true
			}
		});

		txR.writer.text = text;
	}
}

Button::~Button()
{
	txt_.Destroy();
	self_.Destroy();
}

SDL_FRect Button::GetButtonBoundingBox() const
{
	auto [tf, r] = self_.GetComponents<Transform, SpriteRenderableComponent>();

	float w = r.sprite.plot.rect.w * tf.scale.x;
	float h = r.sprite.plot.rect.h * tf.scale.y;

	return { tf.position.x - (w / 2.0f), tf.position.y - (h / 2.0f), w, h };
}

void Button::AdjustTextYPos(float yAdjust)
{
	assert(txt_.IsValid());

	auto& selfTf = self_.GetComponent<Transform>();
	auto& txtTf = txt_.GetComponent<Transform>();
	txtTf.position.y = (selfTf.position.y + yAdjust) * txtTf.scale.y;
}



Result<Button::ButtonSprites> 
Button::LoadButtonSprites(SDL_Renderer* renderer, NewTextureRepository& repo, 
						  std::string_view buttonUpFilename, 
						  std::string_view buttonDownFilename)
{
	ButtonSprites sprites;

	TRY(ResourcePath::Sprite(buttonUpFilename), btnUpFilepath);
	TRY(ResourcePath::Sprite(buttonDownFilename), btnDownFilepath);

	assert(renderer);
	TRY(SpriteAtlas::Create(renderer), spriteAtlas);

	TRY_ASSIGN(sprites.up, spriteAtlas.LoadSprite(renderer, {
		.spriteName = std::string{kButtonUpSpriteName},
		.filepath = btnUpFilepath }));
	TRY_ASSIGN(sprites.down, spriteAtlas.LoadSprite(renderer, {
		.spriteName = std::string{kButtonDownSpriteName},
		.filepath = btnDownFilepath }));

	TRY(repo.AttachAtlas(std::move(spriteAtlas)));

	return sprites;
}

} // ui