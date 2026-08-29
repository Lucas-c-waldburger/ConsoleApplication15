#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "../../Fixtures.h"

namespace ui {

enum class ComponentBuilderType
{
	RigidBody = 0,
	Collider,
	EventCallback
};

inline constexpr ComponentBuilderType kInvalidComponentBuilderType =
	static_cast<ComponentBuilderType>(-1);

class IEditorComponentBuilder
{
public:
	virtual ~IEditorComponentBuilder() = default;

	virtual bool Draw(Entity& e, SceneFixture& fixture) = 0;
	void SetIsActive(bool val)
	{
		if (isActive_ == val) { return; }

		SetIsActiveImpl(val);

		isActive_ = val;
	}

	virtual Result<Void> Init(SceneFixture& fixture) { return kVoid; }

	ComponentBuilderType GetBuilderType() const noexcept { return builderType_; }
	bool IsActive() const noexcept { return isActive_; }

protected:
	explicit IEditorComponentBuilder(ComponentBuilderType type) : builderType_(type) {}
	virtual void SetIsActiveImpl(bool val) = 0;

private:
	ComponentBuilderType builderType_;
	bool isActive_ = false;
};

} // ui

#endif