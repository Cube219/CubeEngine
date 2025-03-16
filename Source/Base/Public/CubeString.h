#pragma once

#include <string>
#include <string_view>

#include "Types.h"

#define CUBE_DEFAULT_STRING_UTF16

namespace cube
{
    using AnsiCharacter = char;
    using AnsiString = std::basic_string<AnsiCharacter>;
    using AnsiStringView = std::basic_string_view<AnsiCharacter>;

    using U8Character = char8_t;
    using U8String = std::basic_string<U8Character>;
    using U8StringView = std::basic_string_view<U8Character>;

    using U16Character = char16_t;
    using U16String = std::basic_string<U16Character>;
    using U16StringView = std::basic_string_view<U16Character>;

    using U32Character = char32_t;
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
#define CUBE_T(text) u8##text

#elif defined(CUBE_DEFAULT_STRING_UTF16)

    using Character = U16Character;
    using String = U16String;
    using StringView = U16StringView;
#define CUBE_T(text) u##text

#elif defined(CUBE_DEFAULT_STRING_UTF32)

    using Character = U32Character;
    using String = U32String;
    using StringView = U32StringView;
#define CUBE_T(text) U##text

#else

#error You must define one of string type

#endif
} // namespace cube
