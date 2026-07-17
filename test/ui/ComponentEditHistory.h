#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "../../ecs/Ecs.h"
#include "gui_edit/GuiComponentNames.h"
#include <deque>

namespace ui {

enum class Action
{
	AddComponent,
	RemoveComponent,
	EditComponent,
	AddEntity,
	RemoveEntity
};

struct TypeErasedRecord
{
	Entity_t entity;
	
	fu2::unique_function<void(Entity&)> undo;
	fu2::unique_function<void(Entity&)> redo;
};

namespace detail {

template <Action action> struct make_record;

template <> struct make_record<Action::AddComponent> 
{
	template <typename T>
	static TypeErasedRecord call(Entity_t id, const T& cmp) 
	{
		using Type = std::remove_cvref_t<T>;

		return TypeErasedRecord{
			.entity = id,
			.undo = +[](Entity& e) {
				assert(e.HasComponent<Type>());
				e.RemoveComponent<Type>();
			},
			.redo = [cmp](Entity& e) {
				assert(!e.HasComponent<Type>());
				e.AddComponent<Type>() = cmp;
			}
		};
	}
};

template <> struct make_record<Action::RemoveComponent> 
{
	template <typename T>
	static TypeErasedRecord call(Entity_t id, const T& cmp) 
	{
		using Type = std::remove_cvref_t<T>;

		return TypeErasedRecord{
			.entity = id,
			.undo = [cmp](Entity& e) {
				assert(!e.HasComponent<Type>());
				e.AddComponent<Type>() = cmp;
			},
			.redo = +[](Entity& e) {
				assert(e.HasComponent<Type>());
				e.RemoveComponent<Type>();
			}
		};
	}
};

template <> struct make_record<Action::EditComponent> 
{
	template <typename T>
	static TypeErasedRecord call(Entity_t id, const T& before, const T& after) 
	{
		using Type = std::remove_cvref_t<T>;

		return TypeErasedRecord{
			.entity = id,
			.undo = [before](Entity& e) {
				assert(e.HasComponent<Type>());
				e.AddComponent<Type>() = before;
			},
			.redo = [after](Entity& e) {
				assert(e.HasComponent<Type>());
				e.AddComponent<Type>() = after;
			}
		};
	}
};

} // detail

template <Action action, typename T, typename...Args>
inline TypeErasedRecord MakeRecord(Entity_t e, Args&&...args)
{
	return detail::make_record<action>::template call<T>(e, std::forward<Args>(args)...);
}

template <typename T>
struct GuiRedoUndoableComponentPred : std::bool_constant<
	(public_mutable_component_v<T> && HasGuiComponentName<T> && !std::same_as<T, Name>)
>{};

using GuiRedoUndoableComponentTypeList = 
	filter_types_t<CoreComponentTypeList, GuiRedoUndoableComponentPred>;

class ComponentEditHistory
{
public:
	template <typename T>
	static constexpr bool supported_component_v = type_in_list_v<T, GuiRedoUndoableComponentTypeList>;

	static constexpr size_t kMaxRecords = 50;

	template <typename T>
	static void PushAddComponent(Entity& e, const T& cmp)
	{
		if constexpr (supported_component_v<T>)
		{
			PushAction<Action::AddComponent, T>(e, cmp);
		}
	}

	template <typename T>
	static void PushRemoveComponent(Entity& e, const T& cmp)
	{
		if constexpr (supported_component_v<T>)
		{
			PushAction<Action::RemoveComponent, T>(e, cmp);
		}	
	}

	template <typename T>
	static void BeginComponentEdit(Entity& e, const T& cmp)
	{
		if constexpr (supported_component_v<T>)
		{
			if (!e.IsValid())
			{
				return;
			}

			auto& cmpAtBegin = componentEditTracker_.GetComponentAtEditBegin<T>();

			if (!cmpAtBegin.has_value())
			{
				cmpAtBegin = cmp;
				componentEditTracker_.entity = e.GetID();
			}
		}
	}

	template <typename T>
	static void EndComponentEdit(Entity& e, const T& cmp)
	{
		if constexpr (supported_component_v<T>)
		{
			auto& cmpAtBegin = componentEditTracker_.GetComponentAtEditBegin<T>();

			if (e.IsValid() && e.GetID() == componentEditTracker_.entity)
			{
				if (cmpAtBegin.has_value())
				{
					if (*cmpAtBegin != cmp)
					{
						PushAction<Action::EditComponent, T>(e, *cmpAtBegin, cmp);
					}
				}
			}

			cmpAtBegin.reset();
			componentEditTracker_.entity = kInvalidEntity;
		}  
	}

	static void Undo();

	static void Redo();

	static bool Empty();
	static void Reset();

	static bool CanUndo();
	static bool CanRedo();

	static int GetCursor()
	{
		return cursor_;
	}

	static size_t GetRecordsSize()
	{
		return records_.size();
	}

	static Entity_t GetEntityForCurrentRecord();

private:
	static constexpr Entity_t kUnresolvedEntity = kInvalidEntity - 1;

	struct ComponentEditTracker
	{
		using ComponentAtEditBegin = GuiRedoUndoableComponentTypeList::AsTuple<std::optional>;

		template <typename T>
		std::optional<T>& GetComponentAtEditBegin()
		{
			return std::get<std::optional<T>>(componentAtEditBegin);
		}
		template <typename T>
		const std::optional<T>& GetComponentAtEditBegin() const
		{
			return std::get<std::optional<T>>(componentAtEditBegin);
		}

		ComponentAtEditBegin componentAtEditBegin;
		Entity_t entity = kInvalidEntity;
	};

	static bool CursorValid();
	static bool CursorAtHead();
	static bool CursorAtEarliest();
	static void SetCursorAsHead();
	static bool CursorEarlierThanHead();

	template <Action action, typename T, typename...Args>
	static void PushAction(Entity& e, Args&&...args)
	{
		if (CursorEarlierThanHead())
		{
			SetCursorAsHead();
		}

		if (!e.IsValid())
		{
			return;
		}

		assert(records_.size() <= kMaxRecords);
		if (records_.size() == kMaxRecords)
		{
			assert(cursor_ == static_cast<int>(kMaxRecords) - 1);    

			records_.pop_front();
			--cursor_;
		}

		const auto id = e.GetID();

		records_.emplace_back(MakeRecord<action, T>(id, std::forward<Args>(args)...));

		++cursor_;
	}

	static int RecordsSize() { return static_cast<int>(records_.size()); }

	static inline std::deque<TypeErasedRecord> records_{};
	static inline int cursor_ = -1;
	static inline ComponentEditTracker componentEditTracker_{};
};

} // ui



#endif