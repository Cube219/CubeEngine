#pragma once

#ifdef CUBE_PLATFORM_MACOS

#include "CoreHeader.h"

#include <Foundation/Foundation.h>

#include "CubeString.h"
#include "Format.h"

namespace cube
{
#define MACOS_T(text) text
    using MacOSCharacter = char;
    using MacOSString = std::basic_string<MacOSCharacter>;
    using MacOSStringView = std::basic_string_view<MacOSCharacter>;

    using FrameMacOSString = TFrameString<MacOSCharacter>;

    namespace string_internal
    {
#if CUBE_SUPPORT_UNICODE_CHARACTER
        template <>
        CUBE_CORE_EXPORT Uint32 DecodeCharacterAndMove(const MacOSCharacter*& pStr);

        template <>
        CUBE_CORE_EXPORT int EncodeCharacterAndAppend(Uint32 code, MacOSCharacter* pStr);
#endif

        // String -> NSString
        template <StringViewable SrcStringViewable>
        struct Converter<NSString*, SrcStringViewable>
        {
            static constexpr bool Available = true;
            static constexpr bool NeedConvert = true;

            // ARC: reference-to-pointer parameters default to __autoreleasing; explicitly use __strong
            // to match __strong locals (e.g. the 'res' temporary inside String_Convert).
            static void ConvertAndAppend(NSString* __strong& dst, const SrcStringViewable& src)
            {
                // Use the format scratch allocator.
                FrameAllocator& formatScratchAllocator = format::internal::GetMyThreadFormatScratchAllocator();
                FrameAllocationScope scope(formatScratchAllocator);

                FrameAllocator::StdAllocator<U8Character> scratchAllocator(formatScratchAllocator);
                TFrameString<U8Character> u8Src(scratchAllocator);
                String_ConvertAndAppend(u8Src, src);

                dst = [NSString stringWithUTF8String:u8Src.data()];
            }
        };

        // NSString -> String
        template <StringType DstString>
        struct Converter<DstString, NSString*>
        {
            static constexpr bool Available = true;
            static constexpr bool NeedConvert = true;

            static void ConvertAndAppend(DstString& dst, const NSString* src)
            {
                String_ConvertAndAppend(dst, U8StringView([src UTF8String]));
            }
        };
    } // namespace string_internal
} // namespace cube

#endif // CUBE_PLATFORM_MACOS
