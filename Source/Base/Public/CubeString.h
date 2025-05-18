#pragma once

#include <string>
#include <string_view>
#include <type_traits>

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
        // Traits
        // IsCharacter
        template <typename T, typename Enabled = void>
        struct IsCharacter : std::false_type {};
        template <typename T>
        struct IsCharacter<T,
            std::enable_if_t<
                std::is_same_v<T, AnsiCharacter> ||
                std::is_same_v<T, U8Character> ||
                std::is_same_v<T, U16Character> ||
                std::is_same_v<T, U32Character>
            >
        > : std::true_type {};

        // IsString
        template <typename T, typename Enabled = void>
        struct IsString : std::false_type {};
        template <typename T>
        struct IsString<T,
            std::enable_if_t<std::is_same_v<T, std::basic_string<typename T::value_type, typename T::traits_type, typename T::allocator_type>>>
        > : std::true_type {};

        // IsStringView
        template <typename T, typename Enabled = void>
        struct IsStringView : std::false_type {};
        template <typename T>
        struct IsStringView<T,
            std::enable_if_t<std::is_same_v<T, std::basic_string_view<typename T::value_type, typename T::traits_type>>>
        > : std::true_type {};

        // ToStringView
        template <typename Char>
        std::enable_if_t<IsCharacter<Char>::value,
            std::basic_string_view<Char>
        > ToStringView(const Char* srcCStr)
        {
            return std::basic_string_view<Char>(srcCStr);
        }
        template <typename String>
        std::enable_if_t<IsString<String>::value,
            std::basic_string_view<typename String::value_type>
        > ToStringView(const String& srcString)
        {
            return std::basic_string_view<typename String::value_type>(srcString);
        }
        template <typename StringView>
        std::enable_if_t<IsStringView<StringView>::value,
            std::basic_string_view<typename StringView::value_type>
        > ToStringView(const StringView& srcString)
        {
            return std::basic_string_view<typename StringView::value_type>(srcString);
        }

        // IsStringViewable
        template <typename T, typename Enabled = void>
        struct IsStringViewable : std::false_type {};
        template <typename T>
        struct IsStringViewable<T,
            std::void_t<decltype(ToStringView(std::declval<T>()))>
        > : std::true_type {};

        // Decode/Encode function
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

        template <typename T>
        struct NoEntry : std::false_type {};

        // Converter
        template <typename DstString, typename SrcStr, typename Enabled = void>
        struct Converter
        {
            static constexpr bool Available = false;

            static void ConvertAndAppend(DstString& dst, const SrcStr& src)
            {
                static_assert(NoEntry<DstString>::value, "Not implemented converter.");
            }
        };

        // C-style string, std::string and std::string_view
        template <typename DstString, typename SrcStr>
        struct Converter<DstString, SrcStr,
            std::enable_if_t<
                IsString<DstString>::value &&
                IsStringViewable<SrcStr>::value
            >
        >
        {
            static constexpr bool Available = true;

            static void ConvertAndAppend(DstString& dst, const SrcStr& src)
            {
                auto srcStrView = string_internal::ToStringView(src);

                using SrcChar = typename decltype(srcStrView)::value_type;
                using DstChar = typename DstString::value_type;

                DstChar tempBuffer[8];
                const SrcChar* srcCurrent = srcStrView.data();
                const SrcChar* srcEnd = srcCurrent + srcStrView.size();

                while (srcCurrent != srcEnd)
                {
                    int size = string_internal::ConvertCodeAndMove(srcCurrent, tempBuffer);
                    dst.append(tempBuffer, size);
                }
            }
        };
    } // namespace string_internal

    template <typename DstString, typename SrcStr>
    void String_ConvertAndAppend(DstString& dst, const SrcStr& src)
    {
        string_internal::Converter<DstString, SrcStr>::ConvertAndAppend(dst, src);
    }

    template <typename DstString, typename SrcStr>
    DstString String_Convert(const SrcStr& src)
    {
        DstString res;
        String_ConvertAndAppend<DstString, SrcStr>(res, src);
        return res;
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
