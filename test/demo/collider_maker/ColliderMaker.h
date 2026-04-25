#include "../../../FeatureFlags.h"

#if IMGUI_ENABLED 

#include <imgui.h>
#include "ColliderMakerCommon.h"
#include "../../../ecs/EntityPhysics.h"

namespace test {

class ColliderMakerContext : HasWriteAccessImpl<ColliderMakerContext, B2Body, B2Shape>
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

	class Shapes
	{
	public:
		using ID = Entity_t;

		ID NewShape(B2Shape::Type type, SDL_Color clr = {0,0,0,0});
		bool AddPoint(ID id, SDL_FPoint p);
		bool UndoPoint(ID id);
		bool CanAddMorePoints(ID id) const;
		bool CanBuildShape(ID id) const;
		void FinalizeShape(ID id);
		bool ClearPoints(ID id);
		bool EraseShape(ID id);
		B2Shape::Type GetShapeType(ID id) const;
		SDL_Color GetShapeColor(ID id) const;
		std::vector<SDL_FPoint> GetShapePoints(ID id) const;
		void SetTargetPoint(ID id, std::optional<SDL_FPoint> target);
		bool ShapeValid(ID id) const; 
		std::optional<SDL_FPoint> GetCenter(ID id) const;

		const ShapeData* GetShapeData(ID id) const;

		std::optional<B2ShapeParameters> 
		MakeShapeParameters(ID id, SDL_FPoint bodyPos = {0.0f,0.0f}) const;

		const std::set<ID>& GetShapeIDs() const { return ids_; }

	private:
		ShapeData* GetShapeData(ID id);

		std::set<ID> ids_;
	};

	static Result<Void> AddCollider(Shapes::ID id, Entity& e, 
									const EntityPhysics::ColliderParams& params);

	static inline SceneFixture::WeakPtr scene{};
	static inline Shapes shapes{};
	static inline std::string currentErrorMsg{};

private:
	ColliderMakerContext() = default;
};


class ColliderMakerUI
{
public:
	using ShapeID = ColliderMakerContext::Shapes::ID;

	static Result<Void> Init(SceneFixture::SharedPtr& fixture, Entity& e);

	static void Cleanup();

	static void Draw(Entity& e);

private:
	static bool HasActiveShape();

	static inline ShapeID activeShape_ = kInvalidEntity;
	static inline bool committed_ = false;
};


} // test

#endif
