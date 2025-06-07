#include "EventCallbackMap.h"

//bool EventCallbackMap::Erase(const Handle<EventCallback>& handle)
//{
//	if (!handle.IsValid())
//	{
//		return false;
//	}
//
//	auto it = map_.find(handle.eventType_);
//	if (it == map_.end())
//	{
//		return false;
//	}
//
//	return EraseIf(it->second, [&handle](const auto& cb) {
//		return handle == cb.handle;
//	});
//}
//
////bool EventCallbackMap::SetSignal(const Handle<EventCallback>& handle, ReturnSignal signal)
////{
////	auto it = FindCallback(handle);
////	if (it != kCallbacksEnd)
////	{
////		it->lastSignal = signal;
////		return true;
////	}
////	
////	return false;
////}
//
//EventCallbackMap::CallbackIter EventCallbackMap::FindCallback(const Handle<EventCallback>& handle)
//{
//	if (!handle.IsValid())
//	{
//		return kCallbacksEnd;
//	}
//
//	auto it = map_.find(handle.eventType_);
//	if (it == map_.end())
//	{
//		return kCallbacksEnd;
//	}
//
//	return FindIf(it->second, [&handle](const auto& cb) {
//		return cb.handle == handle;
//	});
//}
