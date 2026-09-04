#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "../../Fixtures.h"

namespace ui {

enum class PanelType : size_t
{
	Entities = 0,
	Systems,
	Components,
	Events
};

static constexpr PanelType kInvalidPanelType = 
	static_cast<PanelType>(std::numeric_limits<PanelType>::max());

//class IEditorPanel
//{
//public:
//	virtual ~IEditorPanel() = default;
//
//	virtual void Update(Entity e, SceneFixture& fixture) = 0;
//	virtual Result<Void> Init(SceneFixture& fixture) = 0;
//	virtual void ClearState() {};
//	virtual void TearDown() {}
//
//	PanelType GetPanelType() const noexcept { return panelType_; }
//
//protected:
//	explicit IEditorPanel(PanelType type) : panelType_(type) {}
//
//private:
//	PanelType panelType_ = kInvalidPanelType;
//};

template <typename Derived, PanelType ptype>
class BaseEditorPanel
{
public:
	void Update(Entity e, SceneFixture& fixture)
	{
		static_cast<Derived*>(this)->UpdateImpl(e, fixture);
	};
	Result<Void> Init(SceneFixture& fixture)
	{
		return static_cast<Derived*>(this)->InitImpl(fixture);
	}
	void ClearState() 
	{
		static_cast<Derived*>(this)->ClearStateImpl();
	};
	void TearDown() 
	{
		static_cast<Derived*>(this)->TearDownImpl();
	}

	static constexpr PanelType GetPanelType() noexcept { return ptype; }
};

template <typename T>
concept SomeEditorPanel = requires(T t, Entity e, SceneFixture& fx) {
	{ t.Update(e, fx) } -> std::same_as<void>;
	{ t.Init(fx) } -> std::same_as<Result<Void>>;
	{ t.ClearState() } -> std::same_as<void>;
	{ T::GetPanelType() } -> std::same_as<PanelType>;
};

template <typename T>
concept PanelHasTearDown = SomeEditorPanel<T> && requires(T t) {
	{ t.TearDown() } -> std::same_as<void>;
};

} // ui

#endif