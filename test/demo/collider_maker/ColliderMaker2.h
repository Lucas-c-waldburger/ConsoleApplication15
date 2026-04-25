#pragma once
#include "../../../FeatureFlags.h"

#if IMGUI_ENABLED 

#include <imgui.h>
#include "ColliderMakerCommon.h"
#include "../../../ecs/EntityPhysics.h"
#include "../../../deps/nlohmann/json.hpp"

namespace test {

class ColliderMaker2Context
{
public:
	class Mouse
	{
	public:
		static SDL_FPoint GetPos();
		static bool IsLeftClicked();
		static bool IsLeftReleased();
		static bool IsRightClicked();
		static bool IsRightReleased();
		static bool IsLeftHeld();

	private:
		Mouse() = default;
	};

	static inline SceneFixture::WeakPtr scene{};
	static inline GlyphTextWriter textWriter{};

private:
	ColliderMaker2Context() = default;
};

class ShapeManager
{
public:
	struct Shape
	{
		std::string_view name;
		Entity drawEntity;
		std::vector<Entity> pointTextEntities;
	};

	struct FocusState
	{
		bool isFocus = false;
		size_t oldOrdinal = std::numeric_limits<size_t>::max();

		constexpr bool NewFocusThisFrame() const { return oldOrdinal != std::numeric_limits<size_t>::max(); }
	};;

	static constexpr SDL_Color kTextColor = SDLite::kColorBlack;

	bool NewShape(std::string_view name, B2Shape::Type type, SDL_Color clr);
	bool AddPoint(std::string_view name, SDL_FPoint p, const GlyphTextWriter& writer);

	std::vector<std::string_view> GetShapeNames() const;

	void FrameUpdate(std::string_view currentShapeName, const std::optional<SDL_FPoint>& mousePos);

	nlohmann::ordered_json SerializeShape(std::string_view name) const;

	bool HasShape(std::string_view name) const;

	bool CanBuildShape(std::string_view name) const;

	bool CanAddMorePoints(std::string_view name) const;
	
private:
	static Entity MakePointEntity(SDL_FPoint pos, const GlyphTextWriter& writer);

	Shape* GetShape(std::string_view name);
	const Shape* GetShape(std::string_view name) const;

	ShapeData* GetShapeData(std::string_view name);
	const ShapeData* GetShapeData(std::string_view name) const;

	void UpdateShapeData(Shape& shape, const FocusState& state,  
						 const std::optional<SDL_FPoint>& mousePos);

	size_t GetOrdinalIfNewFocus(std::string_view name) const;

	Dictionary<size_t> shapeIdxByName_;
	std::vector<Shape> shapes_;
};

class ColliderMaker2
{
public:
	static constexpr std::string_view kDefaultShapeName = "Shape";
	static constexpr std::string_view kJsonSaveFileName = "collider_maker_shapes.json";
	static constexpr std::string_view kFontFileName = "GoNotoKurrent-Regular.ttf";
	static constexpr int kFontSize = 12;

	static Result<Void> Init(SceneFixture::SharedPtr& fixture);

	static void Draw();

private:
	ColliderMaker2() = default;

	static bool HasActiveShape();

	static Result<Void> SaveShapesToFile(std::string_view fileName);

	static inline ShapeManager shapeManager_{};
	static inline std::string currentShapeName_{};
	static inline bool committed_ = false;
};

} // test


#endif