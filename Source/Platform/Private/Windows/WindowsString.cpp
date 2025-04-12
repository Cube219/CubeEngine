#ifdef CUBE_PLATFORM_WINDOWS

#include "Windows/WindowsString.h"

#include "Logger.h"

namespace cube
{
    namespace string_internal
    {
        template <>
        Uint32 DecodeCharacterAndMove(const WindowsCharacter*& pStr)
        {
            // Same as UTF16 in CubeString.h
            return DecodeCharacterAndMove((const U16Character*&)pStr);
            Uint32 res = 0;

            WindowsCharacter ch = *pStr;

            if ((ch & 0xFC00) == 0xD800)
            {
                // Size: 2
                WindowsCharacter high = ch;
                ch = *(++pStr);
                WindowsCharacter low = ch;
                ++pStr;

                res = (((high & 0x3FF) << 10) | (low & 0x3FF)) + 0x10000;
            }
            else if ((ch & 0xFC00) == 0xD800)
            {
                // Invalid
                CUBE_LOG(LogType::Error, WindowsString, "Invalid UTF16 Character({0}). It is low surrogates.", (int)ch);

                return 0;
            }
            else
            {
                // Size: 1
                res = ch;
                ++pStr;
            }

            return res;
        }

        template <>
        int EncodeCharacterAndAppend(Uint32 code, WindowsCharacter* pStr)
        {
            return EncodeCharacterAndAppend<U16Character>(code, (U16Character*)pStr);
        }
    } // namespace string_internal
} // namespace cube

#endif // CUBE_PLATFORM_WINDOWS
