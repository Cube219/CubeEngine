#pragma once

#ifdef CUBE_PLATFORM_MACOS

#include "PlatformHeader.h"

#include "CubeString.h"

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
} // namespace cube

#endif // CUBE_PLATFORM_MACOS
