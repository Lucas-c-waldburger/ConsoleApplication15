#pragma once
#include "StableSOA.h"

//class UserComponentSOABase {};
//
//template <typename Class, auto...MemberPtrs> requires SOACompatible<Class, MemberPtrs...>
//class DynamicSOA : public UserComponentSOABase, public StableSOA<Class, MemberPtrs...>
//{
//public:
//	bool Erase(size_t index)
//	{
//		if (index >= Size())
//		{
//			return false;
//		}
//
//		size_t backIdx = Size() - 1;
//
//		[&]<std::size_t...Is>(std::index_sequence<Is...>)
//		{
//			(std::swap(std::get<Is>(memberValues_)[index],
//						std::get<Is>(memberValues_[backIdx])),...);
//
//			((std::get<Is>(memberValues_).pop_back()), ...);
//
//		}(std::make_index_sequence<MemberCount>{});
//
//		return true;
//	}
//
//private:
//};