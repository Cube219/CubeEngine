#pragma once

#ifdef CUBE_PLATFORM_MACOS

#include "PlatformHeader.h"

#include <Foundation/Foundation.h>

#include "CubeString.h"
#include "Format.h"

namespace cube
{
#define MACOS_T(text) text
    using MacOSCharacter = char;
    using MacOSString = std::basic_string<MacOSCharacter>;
    using MacOSStringView = std::basic_string_view<MacOSCharacter>;

    namespace string_internal
    {
#if CUBE_SUPPORT_UNICODE_CHARACTER
        template <>
        CUBE_PLATFORM_EXPORT Uint32 DecodeCharacterAndMove(const MacOSCharacter*& pStr);

        template <>
        CUBE_PLATFORM_EXPORT int EncodeCharacterAndAppend(Uint32 code, MacOSCharacter* pStr);
#endif

        // String -> NSString
        template <StringViewable SrcStringViewable>
        struct Converter<NSString*, SrcStringViewable>
        {
            static constexpr bool Available = true;
            static constexpr bool NeedConvert = true;

            static void ConvertAndAppend(NSString*& dst, const SrcStringViewable& src)
            {
                FormatString<U8Character> u8Src;
                String_ConvertAndAppend(u8Src, src);

                dst = [NSString stringWithUTF8String:u8Src.data()];

                format::internal::DiscardFormatAllocations();
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
