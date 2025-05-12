#pragma once

#include "fmt/format.h"
#include "algorithm" // std::copy in xchar.h
#include "fmt/xchar.h"

#include "CubeString.h"
#include "Types.h"

#include <iterator> // std::back_inserter

namespace cube
{
    namespace format
    {
        namespace internal
        {
            void* AllocateFormat(Uint64 n);
            void* AllocateFormat(Uint64 n, Uint64 alignment);

            void DiscardFormatAllocations();
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
            return (T*)format::internal::AllocateFormat(sizeof(T) * n, alignof(T));
        }

        void deallocate(T* p, size_t n)
        {
            // Do nothing
        }
    };
    template <typename Char>
    using FormatString = std::basic_string<Char, std::char_traits<Char>, FormatAllocator<Char>>;

    namespace format
    {
        namespace internal
        {
            // ----- Convert different string types -----
#define CUBE_IS_SAME_STR_TYPE(DstChar, SrcType) (std::is_same<DstChar, typename decltype(fmt::detail::to_string_view(std::declval<SrcType>()))::value_type>::value)
            // Not string type (Does not have the converter)
            template <typename DstChar, typename SrcType>
            inline std::enable_if_t<
                !string_internal::Converter<std::basic_string<DstChar>, SrcType>::Available,
            const SrcType&>
            ConvertIfStringTypeIsDifferent(const SrcType& value)
            {
                return value;
            }

            // Same string type
            template <typename DstChar, typename SrcType>
            inline std::enable_if_t<
                string_internal::Converter<std::basic_string<DstChar>, SrcType>::Available &&
                CUBE_IS_SAME_STR_TYPE(DstChar, SrcType),
            const SrcType&>
            ConvertIfStringTypeIsDifferent(const SrcType& value)
            {
                return value;
            }

            // Different string type
            template <typename DstChar, typename SrcType>
            inline std::enable_if_t<
                string_internal::Converter<std::basic_string<DstChar>, SrcType>::Available &&
                !CUBE_IS_SAME_STR_TYPE(DstChar, SrcType),
            fmt::basic_string_view<DstChar>>
            ConvertIfStringTypeIsDifferent(const SrcType& value)
            {
                // temp string will be deallocated when DiscardFormatAllocations() is called.
                // (At the end of formatting)
                void* tempMem = AllocateFormat(sizeof(FormatString<DstChar>));
                FormatString<DstChar>* temp = new(tempMem) FormatString<DstChar>;

                String_ConvertAndAppend(*temp, string_internal::ToStringView(value));

                fmt::basic_string_view<DstChar> view(temp->data(), temp->size());
                return view;
            }
#undef CUBE_IS_SAME_STR_TYPE

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

                DiscardFormatAllocations();

                return res;
            }
        } // namespace internal
    } // namespace format

    // ----- Format that can use in various characters -----
    template <typename OutputString = String, typename... Args>
    inline OutputString Format(
        std::basic_string_view<typename OutputString::value_type> formatStr,
        const Args&... args)
    {
        using Char = typename OutputString::value_type;
        using Allocator = typename OutputString::allocator_type;

        return format::internal::cube_vformat<Char, Allocator>(fmt::basic_string_view(formatStr.data(), formatStr.size()), format::internal::ConvertIfStringTypeIsDifferent<Char>(args)...);
    }

    // ----- Custom formatter that can use in various characters -----
    template <typename Char>
    struct cube_formatter
    {
        template <typename ParseContext>
        constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

        template <typename FormatContext, typename... Args>
        auto cube_format(FormatContext& ctx, StringView fmt, const Args&... args) const
        {
            auto&& buf = fmt::detail::get_buffer<Char>(ctx.out());
            auto fmtStr = format::internal::ConvertIfStringTypeIsDifferent<Char>(fmt);
            fmt::detail::vformat_to<Char>(buf, fmtStr, fmt::make_format_args<FormatContext>(format::internal::ConvertIfStringTypeIsDifferent<Char>(args)...), {});

            return fmt::detail::get_iterator(buf, ctx.out());
        }
    };
} // namespace cube
