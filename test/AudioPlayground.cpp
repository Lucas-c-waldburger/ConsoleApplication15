#include "AudioPlayground.h"
#include "../components/builder/RigidBodyComponentBuilder.h"
#include "../components/builder/ColliderComponentBuilder.h"
#include "../sdl/SDLUtils.h"

namespace {

SDL_Rect GetButtonBoundingBox(const Entity& btn)
{
	auto [tf, renderable] = btn.GetComponents<Transform, Renderable>();
	auto rect = std::get<SpriteRenderable>(renderable.renderData).sourcePlot.rect;

	float scaledW = rect.w * tf.scale.x;
	float scaledH = rect.h * tf.scale.y;

	return SDL_Rect{
		static_cast<int>((tf.position.x + renderable.profile.offset.x) - (scaledW / 2.0f)),
		static_cast<int>((tf.position.y + renderable.profile.offset.y) - (scaledH / 2.0f)),
		static_cast<int>(scaledW),
		static_cast<int>(scaledH)
	};
}

SDL_FPoint GetButtonScaleForDimensions(const SpriteRenderable& renderable, 
									   Dimensions<int> reqDims)
{
	const auto [_1, _2, w, h] = renderable.sourcePlot.rect;

	const float scaleX = static_cast<float>(reqDims.w) / static_cast<float>(w);
	const float scaleY = static_cast<float>(reqDims.h) / static_cast<float>(h);

	return { scaleX, scaleY };
}

//Transform MakeButtonTransform(const ui::Button::Params& params)
//{
//	Transform tf{};
//	tf.position = params.position;
//
//	const auto [_, _, w, h] = params.sprites.up.sourcePlot.rect;
//
//	const float scaleX = static_cast<float>(params.dimensions.w) / static_cast<float>(w);
//	const float scaleY = static_cast<float>(params.dimensions.h) / static_cast<float>(h);
//
//	tf.scale = { scaleX, scaleY };
//
//	return tf;
//}

SDL_Rect MakeButtonBoundingBox(const ui::Button::Params& params)
{
	return {
		static_cast<int>(params.position.x - (params.dimensions.w / 2.0f)),
		static_cast<int>(params.position.y - (params.dimensions.h / 2.0f)),
		params.dimensions.w,
		params.dimensions.h
	};
}

bool MouseIsColliding(const SDL_Rect& bbox, const events::MouseInput& ev) 
{
	return PointInsideRect(bbox, ev.input.value.cursor.position.absolute);
}

}

auto ui::Button::MakeOnClickCallback()
{
	return [this](const events::MouseInput& ev) {
		if (ev.input.state == InputState::Pressed)
		{
			if (MouseIsColliding(boundingBox_, ev))
			{
				state_.value |= ButtonState::ClickedInside;
				self_.GetComponent<Renderable>().renderData = sprites_.down;
			}
			//else
			//{
			//	state_.value &= ~State::ClickedInside;
			//	self_.GetComponent<Renderable>().renderData = sprites_.up;
			//}
		}
		else if (ev.input.state == InputState::Released)
		{
			if ((state_.value & ButtonState::ClickedInside) &&
				MouseIsColliding(boundingBox_, ev))
			{
				if (callbacks_.onClick)
				{
					callbacks_.onClick();
				}
			}

			state_.value &= ~ButtonState::ClickedInside;
			self_.GetComponent<Renderable>().renderData = sprites_.up;
		}
	};
}

ui::Button::Button(EventBus2& bus, Params&& params) : self_(ECS::CreateEntity()), 
	callbacks_(std::move(params.callbacks)), sprites_(std::move(params.sprites))
{
	self_.AddComponent(Transform{
		.position = params.position,
		.scale = GetButtonScaleForDimensions(sprites_.up, params.dimensions)
	});

	boundingBox_ = MakeButtonBoundingBox(params);

	auto& tks = self_.AddComponent<SignalTokenStorage>().signalTokens;
	tks.emplace_back(
		bus.ConnectToInput(MouseInputSource::LeftButton, MakeOnClickCallback())
	);

	self_.AddComponent(Renderable{
		.renderData = sprites_.up,
		.profile = { .drawOrder = 10 }
	});

	if (!params.text.has_value())
	{
		auto textChild = self_.GetRelations().AddChild();

		params.text->dimensions = params.dimensions;

		textChild.AddComponent(Renderable{
			.renderData = std::move(*params.text),
			.profile = { .drawOrder = 20 }
		});
		 
		textChild.AddComponent<Transform>().position = params.position;
	}
}

void ui::Button::SetPosition(SDL_FPoint pos)
{
	auto& tf = self_.GetComponent<Transform>();

	const auto posDiff = tf.position - pos;
	boundingBox_.x += posDiff.x;
	boundingBox_.y += posDiff.y;

	tf.position = pos;

	auto rel = self_.GetRelations();
	if (rel.HasChildren())
	{
		rel.GetChildren()[0].GetComponent<Transform>().position = pos;
	}
}

void ui::Button::SetDimensions(Dimensions<int> dim)
{

}






//auto& rigid = self_.AddComponent(ComponentBuilder<RigidBody>{}
//.WithBodyParameters({
//	.bodyType = B2Body::Type::Static,
//	.position = params.position
//	})
//	.Build(world)
//	);
//self_.AddComponent(ComponentBuilder<Collider>{}
//.WithShapeParameters({
//	.shapeType = B2Shape::Type::Polygon,
//	.dimensions = Dimensions<float>{ static_cast<float>(params.dimensions.w),
//									 static_cast<float>(params.dimensions.h) },
//	})
//	.WithColliderSettings({
//		.enableEvents = {.sensor = true },
//		.enableCollision = true,
//		.isSensor = true
//		})
//	.Build(rigid.body)
//	);