#pragma once

#ifdef CUBE_PLATFORM_WINDOWS

#include "CoreHeader.h"

#include "Allocator/FrameAllocator.h"

namespace cube
{
#define WINDOWS_T(text) L ## text
    using WindowsCharacter = wchar_t;
    using WindowsString = std::basic_string<WindowsCharacter>;
    using WindowsStringView = std::basic_string_view<WindowsCharacter>;

    using FrameWindowsString = TFrameString<WindowsCharacter>;

    namespace string_internal
    {
        template <>
        CUBE_CORE_EXPORT Uint32 DecodeCharacterAndMove(const WindowsCharacter*& pStr);

        template <>
        CUBE_CORE_EXPORT int EncodeCharacterAndAppend(Uint32 code, WindowsCharacter* pStr);
    } // namespace string_internal
} // namespace cube

#endif // CUBE_PLATFORM_WINDOWS
