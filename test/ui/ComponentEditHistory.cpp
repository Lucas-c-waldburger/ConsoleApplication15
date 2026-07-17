#include "ComponentEditHistory.h"

#if IMGUI_ENABLED

namespace ui {

void ComponentEditHistory::Undo()
{
	if (!CanUndo())
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

void ComponentEditHistory::Redo()
{
	if (!CanRedo())
	{
		return;
	}

	++cursor_;

	auto& record = records_[static_cast<size_t>(cursor_)];

	auto e = ECS::GetEntityByID(record.entity);
	if (e.IsValid())
	{
		record.redo(e);
	}
}

bool ComponentEditHistory::Empty()
{
	if (records_.empty())
	{
		assert(CursorAtEarliest());
		return true;
	}
	return false;
}

void ComponentEditHistory::Reset()
{
	records_.clear();
	cursor_ = -1;
}

bool ComponentEditHistory::CanUndo()
{
	return !Empty() && !CursorAtEarliest();
}

bool ComponentEditHistory::CanRedo()
{
	return !Empty() && !CursorAtHead();
}

Entity_t ComponentEditHistory::GetEntityForCurrentRecord()
{
	assert(CursorValid());

	return (!Empty() && cursor_ > -1)
		? records_[static_cast<size_t>(cursor_)].entity
		: kInvalidEntity;
}

bool ComponentEditHistory::CursorValid()
{
	return cursor_ >= -1 && cursor_ < RecordsSize();
}

bool ComponentEditHistory::CursorAtHead()
{
	assert(CursorValid());
	return cursor_ == RecordsSize() - 1;
}

bool ComponentEditHistory::CursorAtEarliest()
{
	return cursor_ == -1;
}

void ComponentEditHistory::SetCursorAsHead()
{
	assert(CursorValid());
	records_.resize(cursor_ + 1);
}

bool ComponentEditHistory::CursorEarlierThanHead()
{
	assert(CursorValid());
	return cursor_ < RecordsSize() - 1;
}

} // ui

#endif