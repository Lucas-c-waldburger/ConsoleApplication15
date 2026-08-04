#pragma once
#include <sol/sol.hpp>
#include "../core/Dictionary.h"
#include "../core/Handle.h"
#include "../core/FuncTraits.h"
#include <limits>
#include <cassert>



//struct ScriptResource;
//
//template <>
//class Handle<ScriptResource> : public IHandle<Handle<ScriptResource>>
//{
//public:
//
//private:
//	uint32_t stateIndex_ = std::numeric_limits<uint32_t>::max();
//
//};