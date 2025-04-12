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
    } // namespace string_internal

    template <typename DstStr>
    void String_ConvertAndAppend(DstStr& dst, NSString* srcStr)
    {
        String_ConvertAndAppend(dst, [srcStr UTF8String]) ;
    }
    template <typename SrcStrView>
    NSString* ToNSString(const SrcStrView& src)
    {
        FormatString<U8Character> u8Src;
        String_ConvertAndAppend(u8Src, src);

        NSString* res = [NSString stringWithUTF8String:u8Src.data()];

        format::internal::DiscardFormatAllocations();

        return res;
    }
} // namespace cube

#endif // CUBE_PLATFORM_MACOS
