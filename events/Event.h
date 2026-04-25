#pragma once
#include <cstdint>
#include <limits>
//
inline constexpr uint32_t kInvalidEventType = std::numeric_limits<uint32_t>::max();
//
//struct Event 
//{
//    uint32_t type = kInvalidEventType;
//    uint32_t timestamp = 0;
//    const void* data = nullptr;
//};