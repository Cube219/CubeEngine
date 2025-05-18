#pragma once

#include <array>
#include <cstddef>
#include <limits>
#include <map>
#include <stdint.h>
#include <unordered_map>
#include <vector>

// After Visual Studio 15.8, to use extended alignment, you should define it.
// This will occur when you make shared_ptr with the type of alignment over alignof(max_align_t) using std::make_shared
// (https://developercommunity.visualstudio.com/content/problem/274945/stdmake-shared-is-not-honouring-alignment-of-a.html)
#define _ENABLE_EXTENDED_ALIGNED_STORAGE 1
#include <memory>

namespace cube
{    
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

    using Byte = char;

    using SizeType = size_t;

    constexpr float FLOAT_EPS = std::numeric_limits<float>::epsilon();
    constexpr double DOUBLE_EPS = std::numeric_limits<double>::epsilon();

    template <typename T>
    using SharedPtr = std::shared_ptr<T>;

    template <typename T>
    using UniquePtr = std::unique_ptr<T>;

    template <typename T>
    using WeakPtr = std::weak_ptr<T>;

    template <typename T>
    using Vector = std::vector<T>;

    template <typename Type, size_t Size>
    using Array = std::array<Type, Size>;

    template <typename Key, typename Value>
    using Map = std::map<Key, Value>;

    template <typename Key, typename Value>
    using MultiMap = std::multimap<Key, Value>;

    template <typename Key, typename Value>
    using HashMap = std::unordered_map<Key, Value>;
} // namespace cube
