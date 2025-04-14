#pragma once

#include <string>
#include <string_view>

#include "Defines.h"
#include "Types.h"

#ifndef CUBE_SUPPORT_UNICODE_CHARACTER
#define CUBE_SUPPORT_UNICODE_CHARACTER 1
#endif

#define CUBE_DEFAULT_STRING_UTF16

namespace cube
{
#if CUBE_SUPPORT_UNICODE_CHARACTER

    using AnsiCharacter = char;
#define CUBE_ANSI_T(text) text
    using U8Character = char8_t;
#define CUBE_U8_T(text) u8##text
    using U16Character = char16_t;
#define CUBE_U16_T(text) u##text
    using U32Character = char32_t;
#define CUBE_U32_T(text) U##text

#else // !CUBE_SUPPORT_UNICODE_CHARACTER

    static_assert(sizeof(wchar_t) >= 4, "Size of wchar_t must be greater than or equal 4 if unicode character is not supported.");
    // Ansi will be use UTF8 functions
    // UTF16 will be use UTF32 functions
    
    using AnsiCharacter = char;
#define CUBE_ANSI_T(text) text
    using U8Character = char;
#define CUBE_U8_T(text) text
    using U16Character = wchar_t;
#define CUBE_U16_T(text) L##text
    using U32Character = wchar_t;
#define CUBE_U32_T(text) L##text

#endif // CUBE_SUPPORT_UNICODE_CHARACTER

    using AnsiString = std::basic_string<AnsiCharacter>;
    using AnsiStringView = std::basic_string_view<AnsiCharacter>;

    using U8String = std::basic_string<U8Character>;
    using U8StringView = std::basic_string_view<U8Character>;

    using U16String = std::basic_string<U16Character>;
    using U16StringView = std::basic_string_view<U16Character>;

    using U32String = std::basic_string<U32Character>;
    using U32StringView = std::basic_string_view<U32Character>;

    namespace string_internal
    {
        // Do not implement general template function to identify unspecialized functions in link error.
        template <typename Character>
        Uint32 DecodeCharacterAndMove(const Character*& pStr);

        template <typename Character>
        int EncodeCharacterAndAppend(Uint32 code, Character* pStr);

        template <typename SrcChar, typename DstChar>
        int ConvertCodeAndMove(const SrcChar*& pSrc, DstChar* pDst)
        {
            Uint32 code = DecodeCharacterAndMove(pSrc);
            return EncodeCharacterAndAppend(code, pDst);
        }
    } // namespace string_internal

    template <typename SrcStrView, typename DstStr>
    void String_ConvertAndAppend(DstStr& dst, const SrcStrView& src)
    {
        using SrcChar = typename SrcStrView::value_type;
        using DstChar = typename DstStr::value_type;

        DstChar tempBuffer[8];
        const SrcChar* srcCurrent = src.data();
        const SrcChar* srcEnd = srcCurrent + src.size();

        while (srcCurrent != srcEnd)
        {
            int size = string_internal::ConvertCodeAndMove(srcCurrent, tempBuffer);
            dst.append(tempBuffer, size);
        }
    }

#if defined(CUBE_DEFAULT_STRING_UTF8)

    using Character = U8Character;
    using String = U8String;
    using StringView = U8StringView;
#define CUBE_T(text) CUBE_U8_T(text)

#elif defined(CUBE_DEFAULT_STRING_UTF16)

    using Character = U16Character;
    using String = U16String;
    using StringView = U16StringView;
#define CUBE_T(text) CUBE_U16_T(text)

#elif defined(CUBE_DEFAULT_STRING_UTF32)

    using Character = U32Character;
    using String = U32String;
    using StringView = U32StringView;
#define CUBE_T(text) CUBE_U32_T(text)

#else

#error You must define one of string type

#endif
} // namespace cube
