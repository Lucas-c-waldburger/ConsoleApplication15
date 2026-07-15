#pragma once
#include "../../FeatureFlags.h"

#if IMGUI_ENABLED
#include "InspectorComponentPanel.h"
#include "../../ecs/Ecs.h"
#include <deque>

namespace ui {

////class ComponentEditHistory
////{
////public:
////	template <typename T>
////	static void Push(T&& cmp)
////	{
////		if (currentEntity_ == kInvalidEntity)
////		{
////			return;
////		}
////
////		if (AtSomePreviousState())
////		{
////			SetCurrentIndexAsHead();
////		}
////
////		if (NeedToCycleOutOldHistory())
////		{
////			CycleOutOldHistory();
////		}
////
////		auto& stack = GetStack<T>();
////
////		stack.values.emplace_back(std::forward<T>(cmp));
////		stack.index = stack.values.size() - 1;
////
////		editedComponentSignatures_.emplace_back(T::componentBit);
////
////		++currentIndex_;
////	}
////
////	static bool Undo(Entity& e);
////	static bool Redo(Entity& e);
////
////	static void Clear();
////	static void Reset(Entity_t newE = {});
////
////private:
////	static constexpr size_t kMaxRecords = 50;
////
////	template <typename T>
////	struct ComponentHistoryStack
////	{
////		std::deque<T> values;
////		size_t index = 0;
////
////		void Clear()
////		{
////			values.clear();
////			index = 0;
////		}
////
////		void Reduce()
////		{
////			values.resize(index);
////		}
////	};
////
////	ComponentEditHistory() = default;
////
////	template <typename T>
////	static bool SetComponentOnEntity(Entity& e, ComponentHistoryStack<T>& stack, ComponentSignature bit,
////									 bool increment)
////	{
////		assert(e.IsValid());
////
////		if (T::componentBit != bit)
////		{
////			return true;
////		}
////		if ((!increment && stack.index <= 0) || (increment && stack.index >= stack.values.size()))
////		{
////			return true;
////		}
////
////		stack.index = increment ? stack.index + 1 : stack.index - 1;
////
////		assert(stack.index < stack.values.size());
////
////		e.AddComponent<T>() = stack.values[stack.index];
////
////		return false;
////	}
////
////	template <typename T>
////	static bool RemoveOldComponentEntry(ComponentHistoryStack<T>& stack, ComponentSignature bit)
////	{
////		if (T::componentBit != bit)
////		{
////			return true;
////		}
////
////		assert(!stack.values.empty());
////		assert(stack.index > 0);
////
////		stack.values.pop_front();
////		--stack.index;
////
////		return false;
////	}
////
////	static bool AtSomePreviousState();
////	static bool AtEarliestState();
////	static bool NeedToCycleOutOldHistory();
////	static void CycleOutOldHistory();
////	static void SetCurrentIndexAsHead();
////
////	template <typename T>
////	static void AddComponentForEntity(Entity& e, ComponentHistoryStack<T>& stack)
////	{
////		using Type = std::remove_cvref_t<T>;
////		if (e.HasComponent<Type>())
////		{
////			stack.values.emplace_back(e.GetComponent<Type>());
////		}
////	}
////
////	template <typename T>
////	static ComponentHistoryStack<T>& GetStack()
////	{
////		using Type = std::remove_cvref_t<T>;
////		return std::get<ComponentHistoryStack<T>>(components_);
////	}
////
////	using ComponentHistoryStacks =
////		InspectorComponentPanel::GuiAddableComponentTypeList::AsTuple<ComponentHistoryStack>;
////	
////	static inline ComponentHistoryStacks components_{};
////	static inline std::deque<ComponentSignature> editedComponentSignatures_{};
////	static inline size_t currentIndex_ = 0;
////	static inline Entity_t currentEntity_ = kInvalidEntity;
////};
////



//template <typename T>
//struct Log
//{
//	std::deque<T> values;
//	int index = -1;
//	std::optional<T> editStartValue;
//
//	void Clear()
//	{
//		assert(index < static_cast<int>(values.size()));
//		values.clear();
//		index = -1;
//		editStartValue.reset();
//	}
//
//	void Reduce()
//	{
//		assert(index < static_cast<int>(values.size()));
//		values.resize(index + 1);
//	}
//
//	void PopFront()
//	{
//		assert(!values.empty());
//		assert(index >= 0);
//
//		values.pop_front();
//		--index;
//	}
//};
//
//using ComponentLogStorage = 
//	InspectorComponentPanel::GuiAddableComponentTypeList::AsTuple<Log>;
//
//template <typename TList>
//class component_edit_history_dispatch_tables;
//
//template <template <typename...> class TList, typename...Ts>
//class component_edit_history_dispatch_tables<TList<Ts...>>
//{
//private:
//	template <typename T>
//	static void SetComponentOnEntityImpl(Entity& e, ComponentLogStorage& storage,
//										 bool increment)
//	{
//		assert(e.IsValid());
//
//		auto& log = std::get<ComponentLog<T>>(storage);
//
//		if ((!increment && log.index <= 0) || (increment && log.index + 1 >= log.values.size()))
//		{
//			return;
//		}
//
//		log.index = increment ? log.index + 1 : log.index - 1;
//
//		assert(log.index < log.values.size());
//
//		e.AddComponent<T>() = log.values[log.index];
//	}
//
//	using SetComponentOnEntitySig = void(*)(Entity&, ComponentLogStorage&, bool);
//
//
//	template <typename T>
//	static void PopFrontComponentLogImpl(ComponentLogStorage& storage)
//	{
//		auto& log = std::get<Log<T>>(storage);
//		log.PopFront();
//	}
//
//	using PopFrontComponentLogSig = void(*)(ComponentLogStorage&);
//
//	//template <typename T>
//	//static void AddComponentToEntityImpl(Entity& e, ComponentLogStorage& storage,
//	//									 bool increment)
//	//{
//	//	assert(!e.HasComponent<T>());
//	//
//	//	e.AddComponent<T>();
//	//}
//	
//	template <typename T>
//	static void RemoveComponentFromEntityImpl(Entity& e, ComponentLogStorage& storage,
//											  bool increment)
//	{
//		assert(e.IsValid());
//
//		auto& log = std::get<ComponentLog<T>>(storage);
//
//		if ((!increment && log.index <= 0) || (increment && log.index + 1 >= log.values.size()))
//		{
//			return;
//		}
//
//		log.index = increment ? log.index + 1 : log.index - 1;
//
//		assert(log.index < log.values.size());
//		assert(e.HasComponent<T>());
//	
//		e.RemoveComponent<T>();
//	}
//	
//	using AddRemoveComponentFromEntitySig = void(*)(Entity&);
//
//public:
//	static constexpr size_t size = sizeof...(Ts);
//
//	static constexpr SetComponentOnEntitySig kSetComponentOnEntity[] = {
//		&SetComponentOnEntityImpl<Ts>...
//	};
//	static constexpr PopFrontComponentLogSig kPopFrontComponentLog[] = {
//		&RemoveFrontComponentLogImpl<Ts>...
//	};
//	//static constexpr AddRemoveComponentFromEntitySig kAddComponentToEntity[] = {
//	//	&AddComponentToEntityImpl<Ts>...
//	//};
//	static constexpr SetComponentOnEntitySig kRemoveComponentFromEntity[] = {
//		&RemoveComponentFromEntityImpl<Ts>...
//	};
//};
//
//class ComponentEditHistory
//{
//public:
//	static constexpr size_t kMaxRecords = 50;
//
//	using DispatchTable =
//		component_edit_history_dispatch_tables<InspectorComponentPanel::GuiAddableComponentTypeList>;
//
//	enum class Action 
//	{
//		AddComponent = 1,
//		RemoveComponent,
//		EditComponent 
//	};
//
//	template <typename T>
//	static void PushAdd(const T& cmp) // cmp at time of add (default constructed probs)
//	{
//		PushAction(cmp, Action::AddComponent);
//	}
//
//	template <typename T>
//	static void PushRemove(const T& cmp) // cmp at time of removal
//	{
//		PushAction(cmp, Action::RemoveComponent);
//	}
//
//	template <typename T>
//	static void PushEdit(const T& cmp)
//	{
//		PushAction(cmp, Action::EditComponent);
//	}
//
//	static bool Undo(Entity& e)
//	{
//		if (!CanUndo(e))
//		{
//			return false;
//		}
//
//		--currentIndex_;
//
//		assert(currentIndex_ < records_.size());
//
//		const auto& prevRecord = records_[currentIndex_];
//		assert(prevRecord.componentIndex < DispatchTable::size);
//
//		switch (prevRecord.action)
//		{
//		case Action::AddComponent:
//		case Action::EditComponent:
//			DispatchTable::kSetComponentOnEntity[prevRecord.componentIndex](e, components_, false);
//			break;
//		case Action::RemoveComponent:
//			DispatchTable::kRemoveComponentFromEntity[prevRecord.componentIndex](e, components_, false);
//			break;
//		}
//
//		return true;
//	}
//
//	static bool Redo(Entity& e)
//	{
//		if (!CanRedo(e))
//		{
//			return false;
//		}
//
//		++currentIndex_;
//
//		assert(currentIndex_ < records_.size());
//
//		const auto& nextRecord = records_[currentIndex_];
//		assert(nextRecord.componentIndex < DispatchTable::size);
//
//		switch (nextRecord.action)
//		{
//		case Action::AddComponent:
//		case Action::EditComponent:
//			DispatchTable::kSetComponentOnEntity[nextRecord.componentIndex](e, components_, true);
//			break;
//		case Action::RemoveComponent:
//			DispatchTable::kRemoveComponentFromEntity[nextRecord.componentIndex](e, components_, true);
//			break;
//		}
//
//		return true;
//	}
//
//	template <typename T>
//	void BeginEdit(const T& cmp)
//	{
//		auto& log = GetComponentLog<T>();
//
//		if (!log.editStartValue.has_value())
//		{
//			log.editStartValue = cmp;
//		}
//	}
//
//	template <typename T>
//	void EndEdit(const T& cmp)
//	{
//		auto& log = GetComponentLog<T>();
//
//		if (!log.editStartValue.has_value())
//		{
//			return;
//		}
//
//		if (*log.editStartValue != cmp)
//		{
//			PushEdit(cmp);
//		}
//
//		log.editStartValue.reset();
//	}
//
//
//	void Reset(Entity e = {})
//	{
//		std::apply([](auto&&...logs) {
//			((logs.Clear()), ...);
//		}, components_);
//
//		records_.clear();
//		currentIndex_ = -1;
//		currentEntity_ = e.GetID();
//
//		if (currentEntity_ == kInvalidEntity)
//		{
//			return;
//		}
//
//		InspectorComponentPanel::GuiAddableComponentTypeList::ForEachType([&]<typename T> { 
//			if (e.HasComponent<T>())
//			{
//				auto& log = GetComponentLog<T>();
//				assert(log.values.empty());
//				assert(log.index == -1);
//				
//				log.values.emplace_back(e.GetComponent<T>());
//				log.index = 0;
//			}
//		});
//	}
//
//
//private:
//	struct Record
//	{
//		size_t componentIndex = 0;
//		Action action = static_cast<Action>(0);
//	};
//
//	template <typename T>
//	consteval size_t GetComponentIndex()
//	{
//		return index_of_v<std::remove_cvref_t<T>,
//			InspectorComponentPanel::GuiAddableComponentTypeList>;
//	}
//
//	static bool CanUndo(Entity& e)
//	{
//		if (currentEntity_ != e.GetID())
//		{
//			return false;
//		}
//		if (records_.empty())
//		{
//			assert(currentIndex_ == 0);
//			return false;
//		}
//
//		assert(currentIndex_ < records_.size());
//		return currentIndex_ > 0;
//	}
//	static bool CanRedo(Entity& e)
//	{
//		if (currentEntity_ != e.GetID())
//		{
//			return false;
//		}
//		if (records_.empty())
//		{
//			assert(currentIndex_ == 0);
//			return false;
//		}
//
//		assert(currentIndex_ < records_.size());
//		return currentIndex_ < records_.size() - 1;
//	}
//
//	static bool IndexEarlierThanHead()
//	{
//		return !records_.empty() && currentIndex_ < records_.size() - 1;
//	}
//
//	static bool NeedToCycleOutOldHistory()
//	{
//		assert(!IndexEarlierThanHead());
//		return records_.size() >= kMaxRecords;
//	}
//
//	static void SetCurrentIndexAsHead()
//	{
//		std::apply([](auto&...logs) {
//			(logs.Reduce(), ...);
//		}, components_);
//
//		records_.resize(currentIndex_ + 1);
//	}
//
//	static void CycleOutOldHistory()
//	{
//		assert(!records_.empty());
//		assert(currentIndex_ == kMaxRecords - 1);
//
//		while (records_.size() >= kMaxRecords)
//		{
//			auto& record = records_.front();
//			assert(record.componentIndex < DispatchTable::size);
//
//			DispatchTable::kPopFrontComponentLog[record.componentIndex](components_);
//
//			records_.pop_front();
//		}
//		assert(records_.size() == kMaxRecords - 1);
//
//		--currentIndex_;
//
//		assert(currentIndex_ == kMaxRecords - 2);
//	}
//
//	template <typename T>
//	static Log<T>& GetComponentLog()
//	{
//		return std::get<Log<std::remove_cvref_t<T>>>(components_);
//	}
//
//	template <typename T>
//	void PushAction(const T& cmp, Action action)
//	{
//		if (currentEntity_ == kInvalidEntity)
//		{
//			return;
//		}
//
//		if (IndexEarlierThanHead())
//		{
//			SetCurrentIndexAsHead();
//		}
//
//		if (NeedToCycleOutOldHistory())
//		{
//			CycleOutOldHistory();
//		}
//
//		auto& cmpLog = GetComponentLog<T>();
//		if (!cmpLog.values.empty())
//		{
//			assert(cmpLog.index == cmpLog.values.size() - 1);
//		}
//		else
//		{
//			assert(cmpLog.index == 0);
//		}
//
//		cmpLog.values.emplace_back(cmp);
//		cmpLog.index = cmpLog.values.size() - 1;
//
//		records_.emplace_back(GetComponentIndex<T>(), action);
//		++currentIndex_;
//	}
//
//
//	static inline ComponentLogStorage components_{};
//	static inline std::deque<Record> records_{};
//	static inline int currentIndex_ = -1;
//	static inline Entity_t currentEntity_ = kInvalidEntity;
//};

//namespace detail {
//
//template <typename TList>
//struct optional_any_component;
//
//template <template <typename...> class TList, typename...Ts>
//struct optional_any_component<TList<Ts...>>
//{
//	using type = std::variant<std::optional<Ts>...>;
//};
//
//} // detail
//
//template <typename TList> requires is_type_list_v<TList>
//class OptionalAnyComponent
//{
//public:
//	template <typename T>
//	T* GetIf()
//	{
//		if (auto* op = std::get_if<std::optional<T>>(&component_);
//			op && op->has_value())
//		{
//			return &(*op);
//		}
//		return nullptr;
//	}
//	template <typename T>
//	const T* GetIf() const
//	{
//		if (auto* op = std::get_if<std::optional<T>>(&component_);
//			op && op->has_value())
//		{
//			return &(*op);
//		}
//		return nullptr;
//	}
//	template <typename T>
//	bool HasValue() const
//	{
//		auto* op = std::get_if<std::optional<T>>(&component_);
//
//		return op && op->has_value();
//	}
//
//	template <typename T>
//	T& Get()
//	{
//		return *GetIf<T>();
//	}
//	template <typename T>
//	const T& Get()
//	{
//		return *GetIf<T>();
//	}
//
//private:
//	using DataType = typename detail::optional_any_component<TList>::type;
//	DataType component_;
//};

enum class Action
{
	AddComponent,
	RemoveComponent,
	EditComponent
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
	static TypeErasedRecord call(Entity_t id, T&& cmp) 
	{
		using Type = std::remove_cvref_t<T>;

		return TypeErasedRecord{
			.entity = id,
			.undo = +[](Entity& e) {
				assert(e.HasComponent<Type>());
				e.RemoveComponent<Type>();
			},
			.redo = [c = std::forward<T>(cmp)](Entity& e) {
				assert(!e.HasComponent<Type>());
				e.AddComponent<Type>() = c;
			}
		};
	}
};

template <> struct make_record<Action::RemoveComponent> 
{
	template <typename T>
	static TypeErasedRecord call(Entity_t id, T&& cmp) 
	{
		using Type = std::remove_cvref_t<T>;

		return TypeErasedRecord{
			.entity = id,
			.undo = [c = std::forward<T>(cmp)](Entity& e) {
				assert(!e.HasComponent<Type>());
				e.AddComponent<Type>() = c;
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
	static TypeErasedRecord call(Entity_t id, T&& before, T&& after) 
	{
		using Type = std::remove_cvref_t<T>;

		return TypeErasedRecord{
			.entity = id,
			.undo = [c = std::forward<T>(before)](Entity& e) {
				assert(e.HasComponent<Type>());
				e.AddComponent<Type>() = c;
			},
			.redo = [c = std::forward<T>(after)](Entity& e) {
				assert(e.HasComponent<Type>());
				e.AddComponent<Type>() = c;
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

class ComponentEditHistory
{
public:
	static constexpr size_t kMaxRecords = 50;

	template <typename T>
	static void PushAddComponent(Entity& e, const T& cmp)
	{
		PushAction<T>(Action::AddComponent, e, cmp);
	}

	template <typename T>
	static void PushRemoveComponent(Entity& e, const T& cmp)
	{
		PushAction<T>(Action::RemoveComponent, e, cmp);
	}

	template <typename T>
	static void BeginComponentEdit(Entity& e, const T& cmp)
	{
		if (!e.IsValid())
		{
			return;			
		}

		auto& cmpAtBegin = std::get<std::optional<T>>(componentEditTracker_.componentAtEditBegin);
		if (!cmpAtBegin.has_value())
		{
			assert(componentEditTracker_.entity == kInvalidEntity);

			cmpAtBegin = cmp;
			componentEditTracker_.entity = e.GetID();
		}
	}

	template <typename T>
	static void EndComponentEdit(Entity& e, const T& cmp)
	{
		auto& cmpAtBegin = std::get<std::optional<T>>(componentEditTracker_.componentAtEditBegin);

		if (e.IsValid() && e.GetID() == componentEditTracker_.entity)
		{
			if (cmpAtBegin.has_value())
			{
				if (*cmpAtBegin != cmp)
				{
					PushAction<T>(Action::EditComponent, e, cmpAtBegin, cmp);
				}
			}
		}

		cmpAtBegin.reset();
		componentEditTracker_.entity = kInvalidEntity;
	}

	static void Undo()
	{
		if (Empty() || CursorAtEarliest())
		{
			return;
		}

		auto& record = records_[static_cast<size_t>(cursor_)];

		auto e = ECS::GetEntityByID(record.entity);
		if (e.IsValid())
		{
			record.undo(e);
		}

		--cursor_;
	}

	static void Redo()
	{
		if (Empty() || CursorAtHead())
		{
			return;
		}

		auto& record = records_[static_cast<size_t>(cursor_)];

		auto e = ECS::GetEntityByID(record.entity);
		if (e.IsValid())
		{
			record.redo(e);
		}

		++cursor_;
	}

	static bool Empty()
	{
		if (records_.empty())
		{
			assert(CursorAtEarliest());
			return true;
		}
		return false;
	}

	static void Reset()
	{
		records_.clear();
		cursor_ = -1;
	}

private:
	struct ComponentEditTracker
	{
		using ComponentAtEditBegin =
			InspectorComponentPanel::GuiAddableComponentTypeList::AsTuple<std::optional>;

		ComponentAtEditBegin componentAtEditBegin;
		Entity_t entity = kInvalidEntity;
	};

	static bool CursorValid()
	{
		return cursor_ >= -1 && cursor_ < RecordsSize() - 1;
	}

	static bool CursorAtHead()
	{
		assert(CursorValid());
		return cursor_ == RecordsSize() - 1;
	}

	static constexpr bool CursorAtEarliest()
	{
		return cursor_ == -1;
	}

	static void SetCursorAsHead()
	{
		assert(CursorValid());
		records_.resize(cursor_ + 1);
	}

	static bool CursorEarlierThanHead()
	{
		assert(CursorValid());
		return cursor_ < RecordsSize() - 2;
	}

	template <typename T, typename...Args>
	static void PushAction(Action action, Entity& e, Args&&...args)
	{
		if (!e.IsValid())
		{
			return;
		}

		if (CursorEarlierThanHead())
		{
			SetCursorAsHead();
		}

		assert(records_.size() <= kMaxRecords);
		if (records_.size() == kMaxRecords)
		{
			assert(cursor_ == static_cast<int>(kMaxRecords) - 1);    

			records_.pop_front();
			--cursor_;
		}

		const auto id = e.GetID();

		auto pushRecord = [&]<Action act> {
			records_.push_back(MakeRecord<act, T>(id, std::forward<Args>(args)...));
		};

		switch (action)
		{
		case Action::AddComponent:
			pushRecord.template operator()<Action::AddComponent>();
		case Action::RemoveComponent:
			pushRecord.template operator()<Action::RemoveComponent>();
		case Action::EditComponent:
			pushRecord.template operator()<Action::EditComponent>();
		default:
			assert(false && "Unrecognized Action");
		}

		++cursor_;
	}

	static int RecordsSize() { return static_cast<int>(records_.size()); }

	static inline std::deque<TypeErasedRecord> records_{};
	static inline int cursor_ = -1;
	static inline ComponentEditTracker componentEditTracker_{};
};

} // ui



#endif