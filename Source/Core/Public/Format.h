#pragma once

#include "fmt/format.h"
#include "algorithm" // std::copy in xchar.h
#include "fmt/xchar.h"

#include "CoreHeader.h"

#include "Allocator/FrameAllocator.h"
#include "CubeString.h"
#include "Types.h"

namespace cube
{
    namespace format
    {
        namespace internal
        {
            CUBE_CORE_EXPORT FrameAllocator& GetMyThreadFormatScratchAllocator();

            // ----- Convert different string types -----
            // Not has converter or conversion is not needed (Not string type or same string type)
            template <typename DstChar, typename SrcType>
                requires
                    (!string_internal::Converter<std::basic_string<DstChar>, SrcType>::Available) ||
                    (!string_internal::Converter<std::basic_string<DstChar>, SrcType>::NeedConvert)
            CUBE_DLL_HIDDEN const SrcType& ConvertIfStringTypeIsDifferent(const SrcType& value)
            {
                return value;
            }

            // Has converter and conversion is needed
            template <typename DstChar, typename SrcType>
                requires
                    string_internal::Converter<std::basic_string<DstChar>, SrcType>::Available &&
                    string_internal::Converter<std::basic_string<DstChar>, SrcType>::NeedConvert
            CUBE_DLL_HIDDEN TFrameString<DstChar> ConvertIfStringTypeIsDifferent(const SrcType& value)
            {
                FrameAllocator::StdAllocator<DstChar> scratchAllocator(GetMyThreadFormatScratchAllocator());
                TFrameString<DstChar> converted(scratchAllocator);

                String_ConvertAndAppend(converted, value);

                return converted;
            }

            // ----- Custom format functions -----
            template <typename Char>
            using custom_memory_buffer = fmt::basic_memory_buffer<Char, fmt::inline_buffer_size, FrameAllocator::StdAllocator<Char>>;

            // fmt::vformat that support custom character and allocator
            template <typename Char, typename StringAllocator, typename... Args>
            CUBE_DLL_HIDDEN inline std::basic_string<Char, std::char_traits<Char>, StringAllocator> cube_vformat(
                fmt::basic_string_view<Char> format_str,
                const Args&... args)
            {
                auto buffer = custom_memory_buffer<Char>(FrameAllocator::StdAllocator<Char>(GetMyThreadFormatScratchAllocator()));
                fmt::detail::vformat_to(buffer, format_str, fmt::make_format_args<fmt::buffered_context<Char>>(args...));

                return std::basic_string<Char, std::char_traits<Char>, StringAllocator>(buffer.data(), buffer.size());
            }
        } // namespace internal
    } // namespace format

    // ----- Format that can use in various characters -----
    template <typename OutputString = String, typename... Args>
    CUBE_DLL_HIDDEN inline OutputString Format(
        std::basic_string_view<typename OutputString::value_type> formatStr,
        const Args&... args)
    {
        using Char = typename OutputString::value_type;
        using Allocator = typename OutputString::allocator_type;

        FrameAllocationScope scope(format::internal::GetMyThreadFormatScratchAllocator());

        return format::internal::cube_vformat<Char, Allocator>(fmt::basic_string_view(formatStr.data(), formatStr.size()), format::internal::ConvertIfStringTypeIsDifferent<Char>(args)...);
    }

    // ----- Custom formatter that can use in various characters -----
    template <typename Char>
    struct CUBE_DLL_HIDDEN cube_formatter
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

    private:
        // TODO: Remove this if not needed
        // Receive the converted values as lvalue because fmt::make_format_args cannot receive rvalue.
        template <typename FormatContext, typename FormatStr, typename... ConvertedArgs>
        auto cube_format_converted(FormatContext& ctx, const FormatStr& fmtStr, const ConvertedArgs&... args) const
        {
            auto&& buf = fmt::detail::get_buffer<Char>(ctx.out());
            fmt::detail::vformat_to<Char>(buf, fmtStr, fmt::make_format_args<FormatContext>(args...), {});

            return fmt::detail::get_iterator(buf, ctx.out());
        }
    };
} // namespace cube
