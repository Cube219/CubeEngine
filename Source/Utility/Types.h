#pragma once

#include <stdint.h>

using Int8 = int8_t;
using Int16 = int16_t;
using Int32 = int32_t;
using Int64 = int64_t;

using Uint8 = uint8_t;
static constexpr Uint8 Uint8InvalidValue = (Uint8)(-1);
using Uint16 = uint16_t;
static constexpr Uint16 Uint16InvalidValue = (Uint16)(-1);
using Uint32 = uint32_t;
static constexpr Uint32 Uint32InvalidValue = (Uint32)(-1);
using Uint64 = uint64_t;
static constexpr Uint64 Uint64InvalidValue = (Uint64)(-1);

using SizeType = size_t;

// After Visual Studio 15.8, to use extended alignment, you should define it.
// This will occur when you make shared_ptr with the type of alignment over alignof(max_align_t) using std::make_shared
// (https://developercommunity.visualstudio.com/content/problem/274945/stdmake-shared-is-not-honouring-alignment-of-a.html)
#define _ENABLE_EXTENDED_ALIGNED_STORAGE
#include <memory>

#include <utility>
// TODO: Release일 때 전부 RCast로
#define DCast(T) dynamic_cast<T>

#define RCast(T) reinterpret_cast<T>

#define SCast(T) static_cast<T>

#define CCast(T) const_cast<T>

template <typename T>
using SPtr = std::shared_ptr<T>;

template <typename T>
using UPtr = std::unique_ptr<T>;

template <typename T>
using WPtr = std::weak_ptr<T>;

#define DPCast(T) std::dynamic_pointer_cast<T>

#define SPCast(T) std::static_pointer_cast<T>

#define CPCast(T) std::const_pointer_cast<T>

// TODO: 다른 곳으로 옮기기
enum class MouseButtonType
{
    Left, Right, Middle
};

#include "String.h"

#include <vector>
template <typename T>
using Vector = std::vector<T>;

#include <array>
template <typename Type, size_t Size>
using Array = std::array<Type, Size>;

#include <map>
template <typename Key, typename Value>
using Map = std::map<Key, Value>;

template <typename Key, typename Value>
using MultiMap = std::multimap<Key, Value>;

#include <unordered_map>
template <typename Key, typename Value>
using HashMap = std::unordered_map<Key, Value>;
