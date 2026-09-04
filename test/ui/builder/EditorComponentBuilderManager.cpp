#include "EditorComponentBuilderManager.h"

#if IMGUI_ENABLED

namespace ui {

namespace {

constexpr bool ValidBuilderTypeIndex(ComponentBuilderType type)
{
	const auto& idx = magic_enum::enum_index(type);

	return idx.has_value() && idx.value() >= 0 &&
		   idx.value() < std::tuple_size_v<EditorComponentBuilderManager::BuilderArray>;
}

template <typename T, typename...Args>
	requires (std::derived_from<T, IEditorComponentBuilder>&& std::constructible_from<T, Args...>)
void AddBuilder(EditorComponentBuilderManager::BuilderArray& builderArr, Args&&...args)
{
	auto builder = std::make_unique<T>(std::forward<Args>(args)...);
	const auto& builderType = builder->GetBuilderType();

	if (!ValidBuilderTypeIndex(builderType) ||
		builderArr[magic_enum::enum_index(builderType).value()] != nullptr)
	{
		return;
	}

	builderArr[magic_enum::enum_index(builderType).value()] = std::move(builder);
}

} // unnamed


bool EditorComponentBuilderManager::DrawActiveBuilder(Entity& e, SceneFixture& fixture)
{
	if (!ValidBuilderTypeIndex(activeBuilderType_))
	{
		return false;
	}

	auto& builder = builders_[magic_enum::enum_index(activeBuilderType_).value()];
	if (!builder)
	{
		return false;
	}

	return builder->Draw(e, fixture);
}

void EditorComponentBuilderManager::SetActiveBuilder(ComponentBuilderType type)
{
	if (type == activeBuilderType_) { return; }

	if (ValidBuilderTypeIndex(activeBuilderType_))
	{
		auto& currentBuilder = builders_[magic_enum::enum_index(activeBuilderType_).value()];
		if (currentBuilder)
		{
			currentBuilder->SetIsActive(false);
		}
	}

	if (ValidBuilderTypeIndex(type))
	{
		auto& newBuilder = builders_[magic_enum::enum_index(type).value()];
		if (newBuilder)
		{
			newBuilder->SetIsActive(true);
		}
	}

	activeBuilderType_ = type;
}

void EditorComponentBuilderManager::ClearActiveBuilder()
{
	SetActiveBuilder(kInvalidComponentBuilderType);
}

bool EditorComponentBuilderManager::HasActiveBuilder() const noexcept
{
	return ValidBuilderTypeIndex(activeBuilderType_) &&
		   builders_[magic_enum::enum_index(activeBuilderType_).value()] != nullptr;
}

Result<Void> EditorComponentBuilderManager::Init(SceneFixture& fixture)
{
	AddBuilder<EditorRigidBodyBuilder>(builders_);
	AddBuilder<EditorColliderBuilder>(builders_);
	AddBuilder<EditorEventCallbackBuilder>(builders_);

	for (auto& builder : builders_)
	{
		if (builder)
		{
			TRY(builder->Init(fixture));
		}
	}

	return kVoid;
}

} // ui

#endif