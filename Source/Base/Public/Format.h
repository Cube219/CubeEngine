#pragma once

#include "fmt/format.h"

#include "CubeString.h"
#include "Types.h"

#include <iterator> // std::back_inserter

namespace cube
{
    namespace format
    {
        namespace internal
        {
            void* Allocate(size_t n);
            void* Allocate(size_t n, size_t alignment);

            void DiscardAllocations();
        } // namespace internal
    } // namespace format

    template <typename T>
    class FormatAllocator
    {
    public:
        using value_type = T;

        FormatAllocator(const char* pName = nullptr) {}
        template <typename U>
        FormatAllocator(const FormatAllocator<U>& other) noexcept {}

        T* allocate(size_t n)
        {
            return (T*)format::internal::Allocate(sizeof(T) * n, alignof(T));
        }

        void deallocate(T* p, size_t n)
        {
            // Do nothing
        }
    };

    template <typename Char>
    using FormatString = std::basic_string<Char, std::char_traits<Char>, FormatAllocator<Char>>;

    // ----- Convert different string types -----
#define IS_SAME_STR_TYPE(DstChar, SrcType) (std::is_same<DstChar, typename decltype(fmt::detail::to_string_view(std::declval<SrcType>()))::value_type>::value)
    // Not a string
    template <typename DstChar, typename SrcType>
    inline typename std::enable_if<
        !fmt::detail::has_to_string_view<SrcType>::value, const SrcType&>::type
    convert_to_string(const SrcType& value)
    {
        return value;
    }

    // Same string type
    template <typename DstChar, typename SrcType>
    inline typename std::enable_if<
        fmt::detail::has_to_string_view<SrcType>::value &&
        IS_SAME_STR_TYPE(DstChar, SrcType), const SrcType&>::type
    convert_to_string(const SrcType& value)
    {
        return value;
    }

    // Different string type
    template <typename DstChar, typename SrcType>
    inline typename std::enable_if<
        fmt::detail::has_to_string_view<SrcType>::value &&
        !IS_SAME_STR_TYPE(DstChar, SrcType), fmt::basic_string_view<DstChar>>::type
    convert_to_string(const SrcType& value)
    {
        // temp string will be deallocated when format::internal::DiscardAllocations() is called.
        // (At the end of formatting)
        void* tempMem = format::internal::Allocate(sizeof(FormatString<DstChar>));
        FormatString<DstChar>* temp = new(tempMem) FormatString<DstChar>;

        String_ConvertAndAppend(*temp, fmt::detail::to_string_view(value));

        fmt::basic_string_view<DstChar> view(temp->data(), temp->size());
        return view;
    }
#undef IS_SAME_STR_TYPE

    // ----- Custom format functions -----
    template <typename Char>
    using custom_memory_buffer = fmt::basic_memory_buffer<Char, fmt::inline_buffer_size, FormatAllocator<Char>>;

    // fmt::vformat that support custom character and allocator
    template <typename Char, typename StringAllocator, typename... Args>
    inline std::basic_string<Char, std::char_traits<Char>, StringAllocator> cube_vformat(
        fmt::basic_string_view<Char> format_str,
        const Args&... args)
    {
        auto buffer = custom_memory_buffer<Char>();
        fmt::detail::vformat_to(buffer, format_str, fmt::make_format_args<fmt::buffered_context<Char>>(args...));

        std::basic_string<Char, std::char_traits<Char>, StringAllocator> res(buffer.data(), buffer.size());

        format::internal::DiscardAllocations();

        return res;
    }

    template <typename OutputString = cube::String, typename... Args>
    inline OutputString Format(
        std::basic_string_view<typename OutputString::value_type> formatStr,
        const Args&... args)
    {
        using Char = typename OutputString::value_type;
        using Allocator = typename OutputString::allocator_type;

        return cube_vformat<Char, Allocator>(fmt::basic_string_view(formatStr.data(), formatStr.size()), convert_to_string<Char>(args)...);
    }
} // namespace cube
