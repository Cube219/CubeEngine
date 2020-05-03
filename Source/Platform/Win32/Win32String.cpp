#ifdef _WIN32

#include "../PlatformString.h"

#include "../PlatformDebug.h"

namespace cube
{
    namespace internal
    {
        char32_t DecodeAndMoveInPCharacter(const wchar_t*& pStr)
        {
            // Same as UTF16 in String.h
            char32_t res = 0;

            wchar_t ch = *pStr;

            if((ch & 0xFC00) == 0xD800) { // Size: 2
                wchar_t high = ch;
                ch = *(++pStr);
                wchar_t low = ch;
                ++pStr;

                res = (((high & 0x3FF) << 10) | (low & 0x3FF)) + 0x10000;
            }
            else if((ch & 0xFC00) == 0xD800) { // Invalid
                PLATFORM_PRINT_LOG("Invalid UTF16 Character({0}). It is low surrogates.", (int)ch);

                return 0;
            }
            else { // Size: 1
                res = ch;
                ++pStr;
            }

            return res;
        }

        int EncodeInPCharacter(char32_t code, wchar_t* pStr)
        {
            return EncodeInUTF16(code, (char16_t*)pStr);
        }

        // -> PString(wchat_t)
        template <>
        int ConvertCodeAndMove(const char*& pSrc, wchar_t* pDst)
        {
            char32_t code = DecodeAndMoveInUTF8(pSrc);
            return EncodeInPCharacter(code, pDst);
        }

        template <>
        int ConvertCodeAndMove(const char16_t*& pSrc, wchar_t* pDst)
        {
            // wchar is UTF-16LE in Windows. So, just copy.
            *pDst = (wchar_t)*pSrc;
            ++pSrc;

            return 1;
        }

        template <>
        int ConvertCodeAndMove(const char32_t*& pSrc, wchar_t* pDst)
        {
            char32_t code = DecodeAndMoveInUTF32(pSrc);
            return EncodeInPCharacter(code, pDst);
        }

        // PString(wchar_t) ->
        template <>
        int ConvertCodeAndMove(const wchar_t*& pSrc, char* pDst)
        {
            char32_t code = DecodeAndMoveInPCharacter(pSrc);
            return EncodeInUTF8(code, pDst);
        }

        template <>
        int ConvertCodeAndMove(const wchar_t*& pSrc, char16_t* pDst)
        {
            // wchar is UTF-16LE in Windows. So, just copy.
            *pDst = (char16_t)*pSrc;
            ++pSrc;

            return 1;
        }

        template <>
        int ConvertCodeAndMove(const wchar_t*& pSrc, char32_t* pDst)
        {
            char32_t code = DecodeAndMoveInPCharacter(pSrc);
            return EncodeInUTF32(code, pDst);
        }
    } // namespace internal
} // namespace cube

#endif // _WIN32
