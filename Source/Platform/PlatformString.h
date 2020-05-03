#pragma once

#include "PlatformHeader.h"

namespace cube
{
    // TODO: PString 전용 Memory pool allocator 구현

#ifdef _WIN32
#define PLATFORM_T(text) L ## text
    using PCharacter = wchar_t;
    using PString = std::basic_string<PCharacter>;
    using PStringView = std::basic_string_view<PCharacter>;

    namespace internal
    {
        char32_t DecodeAndMoveInPCharacter(const wchar_t*& pStr);
        int EncodeInPCharacter(char32_t code, wchar_t* pStr);
    } // namespace internal
#endif // _WIN32

} // namespace cube
