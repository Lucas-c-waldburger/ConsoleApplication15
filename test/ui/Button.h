#pragma once
#include "../../ecs/Ecs.h"
#include "../../core/SizedEnumMap.h"
#include <string_view>

class EventBus2;
class NewTextureRepository;

namespace ui {

class Button
{
public:
	friend class Workspace;

	using Callback = fu2::unique_function<void()>;

	enum class Color
	{
		Red = 0,
		Blue,
		Yellow,
		Green,
		ENUM_SIZE_
	};
	static_assert(SomeSizedEnum<Color>);

	struct ButtonSprites
	{
		Sprite up;
		Sprite down;
	};

	struct Params
	{
		SDL_FPoint position = { 0.0f, 0.0f };
		Color color = Color::Green;
		Callback onClick = nullptr;
		std::string_view text = "";
		SDL_FPoint scale = { 1.0f, 1.0f };
	};

	static constexpr std::string_view kButtonUpSpriteName = "button_up";
	static constexpr std::string_view kButtonDownSpriteName = "button_down";
	static constexpr std::string_view kButtonUpFilename = "ui/button/button_rectangle_depth_gradient.png";
	static constexpr std::string_view kButtonDownFilename = "ui/button/button_rectangle_gradient.png";
	static constexpr std::string_view kButtonFontFilename = "GoNotoKurrent-Bold.ttf";

	using StringViewPair = std::pair<std::string_view, std::string_view>;

	static inline constexpr SizedEnumMap<Color, StringViewPair> kSpritePathColorMap{
		Color::Green, StringViewPair{kButtonDownFilename, kButtonUpFilename}
	};

	Button(const Handle<Button>& handle, EventBus2& bus, const ButtonSprites& sprites,
		   const GlyphTextWriter& writer, SDL_FPoint pos,
		   Callback&& onClick, std::string_view text, SDL_FPoint scale);

	~Button();

	Button(const Button&) = delete;
	Button& operator=(const Button&) = delete;

	Button(Button&&) noexcept = default;
	Button& operator=(Button&&) noexcept = default;

	Handle<Button> GetHandle() const { return handle_; }

	const ButtonSprites& GetSprites() const { return sprites_; }

	void SetPosition(SDL_FPoint pos)
	{
		SetTFMember(pos, &Transform::position);
	}
	void SetScale(SDL_FPoint sc)
	{
		SetTFMember(sc, &Transform::scale);
	}
	template <typename Fn> requires std::convertible_to<Fn, Callback>
	void SetOnClick(Fn&& fn)
	{
		onClick_ = std::forward<Fn>(fn);
	}

private:
	static Result<ButtonSprites> LoadButtonSprites(
		SDL_Renderer* renderer, NewTextureRepository& repo,
		std::string_view buttonUpFilename, std::string_view buttonDownFilename);

	template <typename T>
	void SetTFMember(T val, T Transform::* member)
	{
		self_.GetComponent<Transform>().*member = val;
		if (txt_.IsValid()) { txt_.GetComponent<Transform>().*member = val; }
	}

	SDL_FRect GetButtonBoundingBox() const;

	void AdjustTextYPos(float yAdjust);

	auto GetButtonPressCallback();

	Handle<Button> handle_;
	Entity self_;
	Entity txt_;
	ButtonSprites sprites_;
	Callback onClick_;
	bool wasPressed_ = false;
};


} // ui