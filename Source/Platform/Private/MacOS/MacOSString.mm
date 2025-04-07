#ifdef CUBE_PLATFORM_MACOS

#include "MacOS/MacOSString.h"

#include "Logger.h"

namespace cube
{
    namespace string_internal
    {
#if CUBE_SUPPORT_UNICODE_CHARACTER
        template <>
        Uint32 DecodeCharacterAndMove(const MacOSCharacter*& pStr)
        {
            // Same as UTF8 in CubeString.h
            return DecodeCharacterAndMove((const U8Character*&)pStr);
        }

        template <>
        int EncodeCharacterAndAppend(Uint32 code, MacOSCharacter* pStr)
        {
            return EncodeCharacterAndAppend<U8Character>(code, (U8Character*)pStr);
        }
#endif
    } // namespace string_internal
} // namespace cube

#endif // CUBE_PLATFORM_MACOS
