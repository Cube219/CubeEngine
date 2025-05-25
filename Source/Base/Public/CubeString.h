#pragma once

#include <concepts>
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

    // Concepts
    template <typename T>
    concept CharacterType =
        std::same_as<T, AnsiCharacter> ||
        std::same_as<T, U8Character> ||
        std::same_as<T, U16Character> ||
        std::same_as<T, U32Character>;

    template <typename T>
    concept StringType =
        std::same_as<T, std::basic_string<typename T::value_type, typename T::traits_type, typename T::allocator_type>>;

    template <typename T>
    concept StringViewType =
        std::same_as<T, std::basic_string_view<typename T::value_type, typename T::traits_type>>;

    namespace string_internal
    {
        // ToStringView
        template <CharacterType SrcCharacter>
        std::basic_string_view<SrcCharacter> ToStringView(const SrcCharacter* srcCStr)
        {
            return std::basic_string_view<SrcCharacter>(srcCStr);
        }
        template <StringType SrcString>
        std::basic_string_view<typename SrcString::value_type> ToStringView(const SrcString& srcString)
        {
            return std::basic_string_view<typename SrcString::value_type>(srcString);
        }
        template <StringViewType SrcStringView>
        std::basic_string_view<typename SrcStringView::value_type> ToStringView(const SrcStringView& srcString)
        {
            return std::basic_string_view<typename SrcStringView::value_type>(srcString);
        }
    } // namespace string_internal

    template <typename T>
    concept StringViewable =
        requires(T v)
        {
            string_internal::ToStringView(v);
        };

    template <typename T1, typename T2>
    concept SameStringViewable =
        StringViewable<T1> &&
        StringViewable<T2> &&
        std::is_same_v<
            typename decltype(ToStringView(std::declval<T1>()))::value_type,
            typename decltype(ToStringView(std::declval<T2>()))::value_type
        >;

    namespace string_internal
    {
        // Decode/Encode function
        // Do not implement general template function to identify unspecialized functions in link error.
        template <typename Character>
        Uint32 DecodeCharacterAndMove(const Character*& pStr);

        template <typename Character>
        int EncodeCharacterAndAppend(Uint32 code, Character* pStr);

        template <typename SrcCharacter, typename DstCharacter>
        int ConvertCodeAndMove(const SrcCharacter*& pSrc, DstCharacter* pDst)
        {
            Uint32 code = DecodeCharacterAndMove(pSrc);
            return EncodeCharacterAndAppend(code, pDst);
        }

        template <typename T>
        struct NoEntry : std::false_type {};

        // Converter
        template <typename Dst, typename Src>
        struct Converter
        {
            static constexpr bool Available = false;
            static constexpr bool NeedConvert = false;

            static void ConvertAndAppend(Dst& dst, const Src& src)
            {
                static_assert(NoEntry<Dst>::value, "Not implemented converter.");
            }
        };

        // C-style string, std::string and std::string_view
        template <StringType DstString, StringViewable SrcStringViewable>
            requires (!SameStringViewable<DstString, SrcStringViewable>)
        struct Converter<DstString, SrcStringViewable>
        {
            static constexpr bool Available = true;
            static constexpr bool NeedConvert = true;

            static void ConvertAndAppend(DstString& dst, const SrcStringViewable& src)
            {
                auto srcStringView = string_internal::ToStringView(src);

                using SrcCharacter = typename decltype(srcStringView)::value_type;
                using DstCharacter = typename DstString::value_type;

                DstCharacter tempBuffer[8];
                const SrcCharacter* srcCurrent = srcStringView.data();
                const SrcCharacter* srcEnd = srcCurrent + srcStringView.size();

                while (srcCurrent != srcEnd)
                {
                    int size = string_internal::ConvertCodeAndMove(srcCurrent, tempBuffer);
                    dst.append(tempBuffer, size);
                }
            }
        };

        // Same string type. Specialize it to avoid static assert error.
        template <StringType DstString, StringViewable SrcStringViewable>
            requires SameStringViewable<DstString, SrcStringViewable>
        struct Converter<DstString, SrcStringViewable>
        {
            static constexpr bool Available = true;
            static constexpr bool NeedConvert = false;

            static void ConvertAndAppend(DstString& dst, const SrcStringViewable& src)
            {
                dst = src;
            }
        };
    } // namespace string_internal

    template <typename Dst, typename Src>
    void String_ConvertAndAppend(Dst& dst, const Src& src)
    {
        string_internal::Converter<Dst, Src>::ConvertAndAppend(dst, src);
    }

    template <typename Dst, typename Src>
    Dst String_Convert(const Src& src)
    {
        Dst res;
        String_ConvertAndAppend<Dst, Src>(res, src);
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
