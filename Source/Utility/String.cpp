#include "String.h"

#include <iostream>

namespace cube
{
    namespace internal
    {
        char32_t DecodeAndMoveInUTF8(const char*& pStr)
        {
            char32_t res = 0;
            char first, second, third, fourth;

            char ch = *pStr;

            if((ch & 0x80) == 0) { // Size: 1
                res = ch;
                ++pStr;
            }
            else if((ch & 0xE0) == 0xC0) { // Size: 2
                first = ch & 0x1F;
                ch = *(++pStr);
                second = ch & 0x3F;
                ++pStr;

                res = (first << 6) | second;
            }
            else if((ch & 0xF0) == 0xE0) { // Size: 3
                first = ch & 0xF;
                ch = *(++pStr);
                second = ch & 0x3F;
                ch = *(++pStr);
                third = ch & 0x3F;
                ++pStr;

                res = (first << 12) | (second << 6) | third;
            }
            else if((ch & 0xF8) == 0xF0) { // Size: 4
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
            else {
                std::wcout << L"String: Invalid UTF8 Character (" << (int)ch << ")." << std::endl;
                return 0;
            }

            return res;
        }

        char32_t DecodeAndMoveInUTF16(const char16_t*& pStr)
        {
            char32_t res = 0;

            char16_t ch = *pStr;

            if((ch & 0xFC00) == 0xD800) { // Size: 2
                char16_t high = ch;
                ch = *(++pStr);
                char16_t low = ch;
                ++pStr;

                res = (((high & 0x3FF) << 10) | (low & 0x3FF)) + 0x10000;
            }
            else if((ch & 0xFC00) == 0xD800) { // Invalid
                std::wcout << L"String: Invalid UTF16 Character (" << ch << "). It is low surrogates." << std::endl;
                return 0;
            }
            else { // Size: 1
                res = ch;
                ++pStr;
            }

            return res;
        }

        char32_t DecodeAndMoveInUTF32(const char32_t*& pStr)
        {
            char32_t code = *pStr;
            ++pStr;

            return code;
        }

        int EncodeInUTF8(char32_t code, char* pStr)
        {
            char first, second, third, fourth;

            if((code & 0xFFFFFF80) == 0) { // Size: 1
                *pStr = (char)code;

                return 1;
            }
            else if((code & 0xFFFFF800) == 0) { // Size: 2
                second = 0x80 | (code & 0x3F);
                code >>= 6;
                first = 0xC0 | (char)code;

                *pStr = (char)first; ++pStr;
                *pStr = (char)second;

                return 2;
            }
            else if((code & 0xFFFF0000) == 0) { // Size: 3
                third = 0x80 | (code & 0x3F);
                code >>= 6;
                second = 0x80 | (code & 0x3F);
                code >>= 6;
                first = 0xE0 | (char)code;

                *pStr = (char)first;  ++pStr;
                *pStr = (char)second; ++pStr;
                *pStr = (char)third;

                return 3;
            }
            else if((code & 0xFFE00000) == 0) { // Size: 4
                fourth = 0x80 | (code & 0x3F);
                code >>= 6;
                third = 0x80 | (code & 0x3F);
                code >>= 6;
                second = 0x80 | (code & 0x3F);
                code >>= 6;
                first = 0xF0 | (char)code;

                *pStr = (char)first;  ++pStr;
                *pStr = (char)second; ++pStr;
                *pStr = (char)third;  ++pStr;
                *pStr = (char)fourth;

                return 4;
            }
            else {
                std::wcout << L"String: Unsupported character in UTF8 (" << (int)code << ")." << std::endl;
                return 0;
            }
        }

        int EncodeInUTF16(char32_t code, char16_t* pStr)
        {
            if((code & 0xFFFF0000) == 0) { // Size: 1
                *pStr = (char16_t)code;

                return 1;
            }
            else { // Size: 2
                code -= 0x10000;
                char16_t high = 0xD800 | (char16_t)(code >> 10);
                char16_t low = 0xDC00 | (code & 0x3FF);

                *pStr = high; ++pStr;
                *pStr = low;

                return 2;
            }
        }

        int EncodeInUTF32(char32_t code, char32_t* pStr)
        {
            *pStr = code;

            return 1;
        }

        // -> UTF8
        template <>
        int ConvertCodeAndMove(const char16_t*& pSrc, char* pDst)
        {
            char32_t code = DecodeAndMoveInUTF16(pSrc);
            return EncodeInUTF8(code, pDst);
        }

        template <>
        int ConvertCodeAndMove(const char32_t*& pSrc, char* pDst)
        {
            char32_t code = DecodeAndMoveInUTF32(pSrc);
            return EncodeInUTF8(code, pDst);
        }

        // -> UTF16
        template <>
        int ConvertCodeAndMove(const char*& pSrc, char16_t* pDst)
        {
            char32_t code = DecodeAndMoveInUTF8(pSrc);
            return EncodeInUTF16(code, pDst);
        }

        template <>
        int ConvertCodeAndMove(const char32_t*& pSrc, char16_t* pDst)
        {
            char32_t code = DecodeAndMoveInUTF32(pSrc);
            return EncodeInUTF16(code, pDst);
        }

        // -> UTF32
        template <>
        int ConvertCodeAndMove(const char*& pSrc, char32_t* pDst)
        {
            char32_t code = DecodeAndMoveInUTF8(pSrc);
            return EncodeInUTF32(code, pDst);
        }

        template <>
        int ConvertCodeAndMove(const char16_t*& pSrc, char32_t* pDst)
        {
            char32_t code = DecodeAndMoveInUTF16(pSrc);
            return EncodeInUTF32(code, pDst);
        }
    } // namespace internal
} // namespace cube
