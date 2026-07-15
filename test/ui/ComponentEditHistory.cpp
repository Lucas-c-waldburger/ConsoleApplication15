#include "ComponentEditHistory.h"

#if IMGUI_ENABLED

namespace ui {

//bool ComponentEditHistory::Undo(Entity& e)
//{
//	if (currentEntity_ != e.GetID())
//	{
//		return false;
//	}
//	if (currentIndex_ == 0)
//	{
//		return false;
//	}
//
//	--currentIndex_;
//
//	assert(currentIndex_ < editedComponentSignatures_.size());
//
//	const auto bit = editedComponentSignatures_[currentIndex_];
//
//	return !std::apply([&](auto&...stacks) {
//		return ((SetComponentOnEntity(e, stacks, bit, false)) && ...);
//	}, components_);
//}
//
//bool ComponentEditHistory::Redo(Entity& e)
//{
//	if (currentEntity_ != e.GetID())
//	{
//		return false;
//	}
//	if (currentIndex_ + 1 >= editedComponentSignatures_.size())
//	{
//		return false;
//	}
//
//	++currentIndex_;
//
//	assert(currentIndex_ < editedComponentSignatures_.size());
//
//	const auto bit = editedComponentSignatures_[currentIndex_];
//
//	return !std::apply([&](auto&...stacks) {
//		return ((SetComponentOnEntity(e, stacks, bit, true)) && ...);
//	}, components_);
//}
//
//void ComponentEditHistory::Clear()
//{
//	std::apply([](auto&...stacks) {
//		(stacks.Clear(), ...);
//	}, components_);
//
//	editedComponentSignatures_.clear();
//	currentIndex_ = 0;
//	currentEntity_ = kInvalidEntity;
//}
//
//void ComponentEditHistory::Reset(Entity_t newE)
//{
//	Clear();
//	currentEntity_ = newE;
//}
//
//bool ComponentEditHistory::AtSomePreviousState()
//{
//	return !editedComponentSignatures_.empty() &&
//		currentIndex_ < editedComponentSignatures_.size() - 1;
//}
//
//bool ComponentEditHistory::AtEarliestState()
//{
//	return !editedComponentSignatures_.empty() && currentIndex_ == 0;
//}
//
//bool ComponentEditHistory::NeedToCycleOutOldHistory()
//{
//	return !AtSomePreviousState() && editedComponentSignatures_.size() >= kMaxRecords;
//}
//
//void ComponentEditHistory::CycleOutOldHistory()
//{
//	while (editedComponentSignatures_.size() > kMaxRecords)
//	{
//		const auto bit = editedComponentSignatures_.front();
//
//		std::apply([&](auto&...stacks) {
//			return ((RemoveOldComponentEntry(stacks, bit)) && ...);
//		}, components_);
//
//		editedComponentSignatures_.pop_front();
//	}
//}
//
//void ComponentEditHistory::SetCurrentIndexAsHead()
//{
//	std::apply([](auto&...stacks) {
//		(stacks.Reduce(), ...);
//	}, components_);
//
//	editedComponentSignatures_.resize(currentIndex_);
//}

} // ui

#endif