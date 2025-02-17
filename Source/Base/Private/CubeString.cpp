#include "CubeString.h"

#include <iostream>

namespace cube
{
    namespace string_internal
    {
        // ===== Ansi =====
        template<>
        Uint32 DecodeCharacterAndMove(const AnsiCharacter*& pStr)
        {
            Uint32 ch = *pStr;
            ++pStr;

            return ch;
        }

        template <>
        int EncodeCharacterAndAppend(Uint32 code, AnsiCharacter* pStr)
        {
            *pStr = (AnsiCharacter)code;

            return 1;
        }

        // ===== UTF8 =====
        template<>
        Uint32 DecodeCharacterAndMove(const U8Character*& pStr)
        {
            Uint32 res = 0;
            U8Character first, second, third, fourth;

            U8Character ch = *pStr;

            if ((ch & 0x80) == 0) // Size: 1
            {
                res = ch;
                ++pStr;
            }
            else if ((ch & 0xE0) == 0xC0) // Size: 2
            {
                first = ch & 0x1F;
                ch = *(++pStr);
                second = ch & 0x3F;
                ++pStr;

                res = (first << 6) | second;
            }
            else if ((ch & 0xF0) == 0xE0) // Size: 3
            {
                first = ch & 0xF;
                ch = *(++pStr);
                second = ch & 0x3F;
                ch = *(++pStr);
                third = ch & 0x3F;
                ++pStr;

                res = (first << 12) | (second << 6) | third;
            }
            else if ((ch & 0xF8) == 0xF0) // Size: 4
            {
                first = ch & 0x7;
                ch = *(++pStr);
                second = ch & 0x3F;
                ch = *(++pStr);
                third = ch & 0x3F;
                ch = *(++pStr);
                fourth = ch & 0x3F;
                ++pStr;

                res = (first << 18) | (second << 12) | (third << 6) | fourth;
            }
            else
            {
                std::wcout << L"CubeString: Invalid UTF8 Character (" << (int)ch << ")." << std::endl;
                return 0;
            }

            return res;
        }

        template <>
        int EncodeCharacterAndAppend(Uint32 code, U8Character* pStr)
        {
            U8Character first, second, third, fourth;

            if ((code & 0xFFFFFF80) == 0) // Size: 1
            {
                *pStr = (U8Character)code;

                return 1;
            }
            else if ((code & 0xFFFFF800) == 0) // Size: 2
            {
                second = 0x80 | (code & 0x3F);
                code >>= 6;
                first = 0xC0 | (U8Character)code;

                *pStr = first;
                ++pStr;
                *pStr = second;

                return 2;
            }
            else if ((code & 0xFFFF0000) == 0) // Size: 3
            {
                third = 0x80 | (code & 0x3F);
                code >>= 6;
                second = 0x80 | (code & 0x3F);
                code >>= 6;
                first = 0xE0 | (U8Character)code;

                *pStr = first;
                ++pStr;
                *pStr = second;
                ++pStr;
                *pStr = third;

                return 3;
            }
            else if ((code & 0xFFE00000) == 0) // Size: 4
            {
                fourth = 0x80 | (code & 0x3F);
                code >>= 6;
                third = 0x80 | (code & 0x3F);
                code >>= 6;
                second = 0x80 | (code & 0x3F);
                code >>= 6;
                first = 0xF0 | (U8Character)code;

                *pStr = first;
                ++pStr;
                *pStr = second;
                ++pStr;
                *pStr = third;
                ++pStr;
                *pStr = fourth;

                return 4;
            }
            else
            {
                std::wcout << L"CubeString: Unsupported character in UTF8 ({" << (int)code << ")." << std::endl;
                return 0;
            }
        }

        // ===== UTF16 =====
        template<>
        Uint32 DecodeCharacterAndMove(const U16Character*& pStr)
        {
            Uint32 res = 0;

            U16Character ch = *pStr;

            if ((ch & 0xFC00) == 0xD800) // Size: 2
            {
                U16Character high = ch;
                ch = *(++pStr);
                U16Character low = ch;
                ++pStr;

                res = (((high & 0x3FF) << 10) | (low & 0x3FF)) + 0x10000;
            }
            else if ((ch & 0xFC00) == 0xD800) // Invalid
            {
                std::wcout << L"CubeString: Invalid UTF16 Character ({" << (int)ch << "). It is low surrogates." << std::endl;
                return 0;
            }
            else // Size: 1
            {
                res = ch;
                ++pStr;
            }

            return res;
        }

        template <>
        int EncodeCharacterAndAppend(Uint32 code, U16Character* pStr)
        {
            if ((code & 0xFFFF0000) == 0) // Size: 1
            {
                *pStr = (U16Character)code;

                return 1;
            }
            else // Size: 2
            {
                code -= 0x10000;
                U16Character high = 0xD800 | (U16Character)(code >> 10);
                U16Character low = 0xDC00 | (code & 0x3FF);

                *pStr = high;
                ++pStr;
                *pStr = low;

                return 2;
            }
        }

        // ===== UTF32 =====
        template<>
        Uint32 DecodeCharacterAndMove(const U32Character*& pStr)
        {
            Uint32 code = *pStr;
            ++pStr;

            return code;
        }

        template <>
        int EncodeCharacterAndAppend(Uint32 code, U32Character* pStr)
        {
            *pStr = (U32Character)code;

            return 1;
        }
    } // namespace string_internal
} // namespace cube
