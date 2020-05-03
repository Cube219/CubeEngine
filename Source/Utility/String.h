#pragma once

#include <string>
#include <string_view>

#define DEFAULT_STRING_UTF16

namespace cube
{
    // Helper class to reference both c-style string and cpp string
    // for using cpp string finally
    // Reference BasicStringRef in fmt library (made by Victor Zverovich)
    // (https://github.com/fmtlib/fmt)
    template <typename C, typename Str>
    class BaseStringRef
    {
    public:
        BaseStringRef(const C* cStr) :
            mStr(cStr), mStrPtr(&mStr)
        {}
        BaseStringRef(const Str& cppStr) :
            mStrPtr(&cppStr)
        {}

        const Str& GetString() const { return *mStrPtr; }
        const C* c_str() const { return mStrPtr->c_str(); }

    private:
        const Str* mStrPtr;
        const Str mStr; // Use when using constructor of C-style string
    };

    namespace internal
    {
        char32_t DecodeAndMoveInUTF8(const char*& pStr);
        char32_t DecodeAndMoveInUTF16(const char16_t*& pStr);
        char32_t DecodeAndMoveInUTF32(const char32_t*& pStr);

        int EncodeInUTF8(char32_t code, char* pStr);
        int EncodeInUTF16(char32_t code, char16_t* pStr);
        int EncodeInUTF32(char32_t code, char32_t* pStr);

        template <typename SrcChar, typename DstChar>
        int ConvertCodeAndMove(const SrcChar*& pSrc, DstChar* pDst);
    } // namespace internal

    template <typename SrcStrView, typename DstStr>
    void String_ConvertAndAppend(DstStr& dst, const SrcStrView& src)
    {
        using SrcChar = typename SrcStrView::value_type;
        using DstChar = typename DstStr::value_type;

        DstChar tempBuffer[8];
        const SrcChar* srcCurrent = src.data();
        const SrcChar* srcEnd = srcCurrent + src.size();

        while(srcCurrent != srcEnd) {
            int size = internal::ConvertCodeAndMove(srcCurrent, tempBuffer);
            dst.append(tempBuffer, size);
        }
    }

    using U8Character = char;
    using U8String = std::basic_string<U8Character>;
    using U8StringView = std::basic_string_view<U8Character>;
    using U8StringRef = BaseStringRef<U8Character, U8String>;

    using U16Character = char16_t;
    using U16String = std::basic_string<U16Character>;
    using U16StringView = std::basic_string_view<U16Character>;
    using U16StringRef = BaseStringRef<U16Character, U16String>;

    using U32Character = char32_t;
    using U32String = std::basic_string<U32Character>;
    using U32StringView = std::basic_string_view<U32Character>;
    using U32StringRef = BaseStringRef<U32Character, U32String>;

    /*
    inline void StringMoveNext(U8String::iterator& iter, size_t offset)
    {
        while(offset > 0) {
            char ch = *iter;
            if((unsigned char)ch <= 127)
                iter += 1;
            else if((ch & 0xE0) == 0xC0)
                iter += 2;
            else if((ch & 0xF0) == 0xE0)
                iter += 3;
            else if((ch & 0xF8) == 0xF0)
                iter += 4;
            else
                throw;

            offset--;
        }
    }
    inline void StringMovePrev(U8String::iterator& iter, size_t offset)
    {
        while(offset > 0) {
            while(1) {
                iter--;

                char ch = *iter;
                if((ch & 0xC0) != 0x80)
                    break;
            }

            offset--;
        }
    }

    inline void StringMoveNext(U16String::iterator& iter, size_t offset)
    {
        while(offset > 0) {
            char16_t ch = *iter;

            if((ch & 0xF800) == 0xD800)
                iter += 2;
            else
                iter += 1;

            offset--;
        }
    }
    inline void StringMovePrev(U16String::iterator& iter, size_t offset)
    {
        while(offset > 0) {
            iter--;

            char16_t ch = *iter;
            if((ch & 0xF800) == 0xD800)
                iter--;

            offset--;
        }
    }

    inline void StringMoveNext(U32String::iterator& iter, size_t offset)
    {
        iter += offset;
    }
    inline void StringMovePrev(U32String::iterator& iter, size_t offset)
    {
        iter -= offset;
    }*/

#if defined (DEFAULT_STRING_UTF8)

    using Character = U8Character;
    using String = U8String;
    using StringView = U8StringView;
    using StringRef = U8StringRef;
#define CUBE_T(text) u8 ## text

#elif defined (DEFAULT_STRING_UTF16)

    using Character = U16Character;
    using String = U16String;
    using StringView = U16StringView;
    using StringRef = U16StringRef;
#define CUBE_T(text) u ## text

#elif defined (DEFAULT_STRING_UTF32)

    using Character = U32Character;
    using String = U32String;
    using StringView = U32StringView;
    using StringRef = U32StringRef;
#define CUBE_T(text) U ## text

#else

#error You must define one of string type

#endif
} // namespace cube
