#pragma once
#include <array>
#include <ranges>
#include <cassert>
#include <SDL_rect.h>
#include "../../ecs/Ecs.h"
#include "../../camera/Camera.h"
#include "../../components/TransformComponent.h"
#include "../../components/RenderableComponent.h"

inline SDL_FRect GetSpriteEntityWorldBox(const Entity& e)
{
	if (!e.IsValid()) 
	{ 
		return { 0.0f, 0.0f, 0.0f, 0.0f }; 
	}
	if (!e.HasComponents<Transform, SpriteRenderableComponent>())
	{
		return { 0.0f, 0.0f, 0.0f, 0.0f };
	}

	const auto [tf, rend] = e.GetComponents<Transform, SpriteRenderableComponent>();
	const auto rect = rend.sprite.plot.rect;

	return {
		tf.position.x - (rect.w * tf.scale.x / 2.0f),
		tf.position.y - (rect.h * tf.scale.y / 2.0f),
		rect.w * tf.scale.x,
		rect.h * tf.scale.y
	};
}

template <size_t nCols, size_t nRows> requires (nCols > 0 && nRows > 0)
struct TileSetDefinition
{
	SDL_FPoint origin = { 0.0f, 0.0f };
	SDL_FPoint scale = { 1.0f, 1.0f };
	std::vector<SpriteRenderableComponent> tileSprites;
	std::array<std::array<size_t, nRows>, nCols> layout;
};

template <size_t nCols, size_t nRows> requires (nCols > 0 && nRows > 0)
class TileSet
{
public:
	enum : uint8_t { Left, Right, Up, Down };

	using EntityMatrix = std::array<std::array<Entity, nRows>, nCols>;

	explicit TileSet(const TileSetDefinition<nCols, nRows>& def)
	{
		assert(!def.tileSprites.empty());

		const size_t firstSpriteIdx = def.layout[0][0];
		assert(firstSpriteIdx < def.tileSprites.size());

		SDL_Rect firstSpriteRect = def.tileSprites[firstSpriteIdx].sprite.plot.rect;

		int tileW = firstSpriteRect.w * def.scale.x;
		int tileH = firstSpriteRect.h * def.scale.y;

		float xPos = def.origin.x + (static_cast<float>(tileW) / 2.0f);
		float startYPos = def.origin.y + (static_cast<float>(tileH) / 2.0f);

		for (size_t c = 0; c < nCols; c++)
		{
			float yPos = startYPos;

			for (size_t r = 0; r < nRows; r++)
			{
				const size_t spriteIdx = def.layout[c][r];
				assert(spriteIdx < def.tileSprites.size());

				const auto& spriteCmp = def.tileSprites[spriteIdx];

				assert(firstSpriteRect.w == spriteCmp.sprite.plot.rect.w);
				assert(firstSpriteRect.h == spriteCmp.sprite.plot.rect.h);

				Entity& e = entityMatrix_[c][r];

				e = ECS::CreateEntity();
				assert(e.IsValid());

				e.AddComponent(Transform{
					.position = { xPos, yPos },
					.scale = def.scale
				});

				e.AddComponent(spriteCmp);

				yPos += tileH;
			}

			xPos += tileW;
		}
	}

	void Update(const Camera& camera);

	SDL_FPoint GetScale() const;
	void SetScale(SDL_FPoint newScale);

	SDL_FPoint GetOrigin() const
	{
		const Entity& e = entityMatrix_[0][0];
		const auto [tf, rend] = e.GetComponents<Transform, SpriteRenderableComponent>();
		const auto rect = rend.sprite.plot.rect;

		return {
			tf.position.x - (rect.w * tf.scale.x / 2.0f),
			tf.position.y - (rect.h * tf.scale.y / 2.0f)
		};
	}

	SDL_FRect GetOriginBoundingBox() const
	{
		const Entity& e = entityMatrix_[0][0];
		const auto [tf, rend] = e.GetComponents<Transform, SpriteRenderableComponent>();
		const auto rect = rend.sprite.plot.rect;

		return {
			tf.position.x - (rect.w * tf.scale.x / 2.0f),
			tf.position.y - (rect.h * tf.scale.y / 2.0f),
			rect.w * tf.scale.x,
			rect.h * tf.scale.y
		};
	}

	SDL_FRect GetTileSetBoundingBox() const
	{
		const Entity& e = entityMatrix_[0][0];
		const auto [tf, rend] = e.GetComponents<Transform, SpriteRenderableComponent>();
		const auto rect = rend.sprite.plot.rect;

		return {
			tf.position.x - (rect.w * tf.scale.x / 2.0f),
			tf.position.y - (rect.h * tf.scale.y / 2.0f),
			rect.w * tf.scale.x * nCols,
			rect.h * tf.scale.y * nRows
		};
	}

private:
	Dimensions<float> GetTileSize() const
	{
		const auto& e = entityMatrix_[0][0];
		const auto [tf, rend] = e.GetComponents<Transform, SpriteRenderableComponent>();
		const auto rect = rend.sprite.plot.rect;

		return {
			.w = rect.w * tf.scale.x,
			.h = rect.h * tf.scale.y
		};
	}

	void Shift(uint8_t dir)
	{
		auto [tileW, tileH] = GetTileSize();
		float tileSetW = tileW * nCols;
		float tileSetH = tileH * nRows;

		bool xShift = (dir == Left || dir == Right);

		if (xShift)
		{
			if (dir == Left)
			{
				std::ranges::rotate(entityMatrix_, entityMatrix_.end() - 1);

				for (size_t r = 0; r < nRows; r++)
				{
					entityMatrix_[0][r].GetComponent<Transform>()
						.position.x -= tileSetW;
				}
			}
			else // Right
			{
				std::ranges::rotate(entityMatrix_, entityMatrix_.begin() + 1);

				for (size_t r = 0; r < nRows; r++)
				{
					entityMatrix_[nCols - 1][r].GetComponent<Transform>()
						.position.x += tileSetW;
				}
			}
		}
		else // yShift
		{
			if (dir == Up)
			{
				for (size_t c = 0; c < nCols; c++)
				{
					std::ranges::rotate(entityMatrix_[c], entityMatrix_[c].end() - 1);

					entityMatrix_[c][0].GetComponent<Transform>()
						.position.y -= tileSetH;
				}
			}
			else // Down
			{
				for (size_t c = 0; c < nCols; c++)
				{
					std::ranges::rotate(entityMatrix_[c], entityMatrix_[c].begin() + 1);

					entityMatrix_[c][nRows - 1].GetComponent<Transform>()
						.position.y += tileSetH;
				}
			}
		}
	}

	EntityMatrix entityMatrix_;
};

template <size_t nCols, size_t nRows> requires (nCols > 0 && nRows > 0)
inline void TileSet<nCols, nRows>::Update(const Camera& camera)
{
	const SDL_FRect vp = camera.GetViewport().GetBoundingBox();
	const auto [tileW, tileH] = GetTileSize();
	const SDL_FRect tileSetBbox = GetTileSetBoundingBox();

	assert(tileSetBbox.w >= vp.w);
	assert(tileSetBbox.h >= vp.h);

	while (vp.x < GetOrigin().x)
	{
		Shift(Left);
	}
	while (vp.x + vp.w > GetOrigin().x + tileW)
	{
		Shift(Right);
	}
	while (vp.y < GetOrigin().y)
	{
		Shift(Up);
	}
	while (vp.y + vp.h > GetOrigin().y + tileH)
	{
		Shift(Down);
	}
}

template<size_t nCols, size_t nRows> requires (nCols > 0 && nRows > 0)
inline SDL_FPoint TileSet<nCols, nRows>::GetScale() const
{
	const Entity& rep = entityMatrix_[0][0];
	assert(rep.IsValid());
	assert(rep.HasComponent<Transform>());

	return rep.GetComponent<Transform>().scale;
}

template<size_t nCols, size_t nRows> requires (nCols > 0 && nRows > 0)
inline void TileSet<nCols, nRows>::SetScale(SDL_FPoint newScale)
{
	SDL_FPoint origin = GetOrigin();
	SDL_FPoint scaleDiff = GetScale() - newScale;

	auto [oldTileW, oldTileH] = GetTileSize();
	auto [newTileW, newTileH] = Dimensions<float>{
		oldTileW * scaleDiff.x,
		oldTileH * scaleDiff.y
	};

	auto [wDiff, hDiff] = Dimensions<float>{
		oldTileW - newTileW,
		oldTileH - newTileH
	};

	for (size_t c = 0; c < nCols; c++)
	{
		for (size_t r = 0; r < nRows; r++)
		{
			auto& pos = entityMatrix_[c][r].GetComponent<Transform>().position;

			pos.x -= wDiff;
			pos.y -= hDiff;
		}
	}
}
