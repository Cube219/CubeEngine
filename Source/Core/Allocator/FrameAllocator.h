#pragma once

#include "../CoreHeader.h"

#include "Utility/Json.h"
#include "Utility/Format.h"

namespace cube
{
    CORE_EXPORT FrameAllocator& GetFrameAllocator();

    class CORE_EXPORT FrameAllocator
    {
    private:
        class MemoryBlock
        {
        public:
            MemoryBlock(Uint64 size);
            ~MemoryBlock();

            MemoryBlock(const MemoryBlock& other) = delete;
            MemoryBlock& operator=(const MemoryBlock& rhs) = delete;

            MemoryBlock(MemoryBlock&& other) noexcept :
                mSize(other.mSize),
                mStartPtr(other.mStartPtr),
                mCurrentPtr(other.mCurrentPtr)
            {
                other.mSize = 0;
                other.mStartPtr = nullptr;
                other.mCurrentPtr = nullptr;
            }
            MemoryBlock& operator=(MemoryBlock&& rhs) noexcept
            {
                mSize = rhs.mSize;
                mStartPtr = rhs.mStartPtr;
                mCurrentPtr = rhs.mCurrentPtr;

                rhs.mSize = 0;
                rhs.mStartPtr = nullptr;
                rhs.mCurrentPtr = nullptr;

                return *this;
            }

            void* Allocate(Uint64 size);
            void* AllocateAligned(Uint64 size, Uint64 alignment);

            void DiscardAllocations();

        private:
            Uint64 mSize;

            void* mStartPtr;
            void* mCurrentPtr;
        };

    public:
        FrameAllocator();
        ~FrameAllocator();

        FrameAllocator(const FrameAllocator& other) = delete;
        FrameAllocator& operator=(const FrameAllocator& rhs) = delete;

        void Initialize(const char* debugName, Uint64 blockSize = 1 * 1024 * 1024); // 1 MiB
        void Shutdown();

        void* Allocate(Uint64 size);
        void Free(void* ptr);

        void* AllocateAligned(Uint64 size, Uint64 alignment);
        void FreeAligned(void* ptr);

        void DiscardAllocations();

    private:
        void AllocateAdditioanlBlock(Uint64 size);

        Uint64 mBlockSize;
        MemoryBlock mMemoryBlock;

        Vector<MemoryBlock> mAdditionalMemBlocks;

#ifdef _DEBUG
        Uint64 mAllocatedSize = 0;
        const char* mDebugName;
#endif // _DEBUG

        /////////////////////////
        // Class for RapidJson //
        /////////////////////////
    public:
        class RapidJsonAllocator
        {
        public:
            RapidJsonAllocator() :
                mAllocator(GetFrameAllocator())
            {}
            RapidJsonAllocator(FrameAllocator& allocator) :
                mAllocator(allocator)
            {}

#ifdef _DEBUG
            // In debug mode, Free() is needed because it checks total allocated size.
            static const bool kNeedFree = true;

            void* Malloc(size_t size)
            {
                // Add an address of the allocator at prefix
                // bacause Free() is static.
                void* ptr = mAllocator.Allocate(size + sizeof(SizeType*));

                SizeType* storedAddress = (SizeType*)ptr;
                *storedAddress = (SizeType)&mAllocator;

                return (Uint8*)ptr + sizeof(SizeType*);
            }

            void* Realloc(void* originalPtr, size_t originalSize, size_t newSize)
            {
                if(originalPtr != nullptr)
                    Free(originalPtr);

                if(newSize == 0)
                    return nullptr;

                return Malloc(newSize);
            }

            static void Free(void* ptr)
            {
                if(ptr == nullptr)
                    return;

                Uint8* realPtr = (Uint8*)ptr - sizeof(SizeType*);

                FrameAllocator* allocator = (FrameAllocator*)(*(SizeType*)realPtr);
                allocator->Free(realPtr);
            }
#else // _DEBUG
            static const bool kNeedFree = false;

            void* Malloc(size_t size)
            {
                return mAllocator.Allocate(size);
            }

            void* Realloc(void* originalPtr, size_t originalSize, size_t newSize)
            {
                if(newSize == 0)
                    return nullptr;

                return Malloc(newSize);
            }

            static void Free(void* ptr)
            {
                // Do nothing
            }
#endif // _DEBUG

        private:
            FrameAllocator& mAllocator;
        };

        ///////////////////
        // Class for Std //
        ///////////////////
    public:
        template <typename T>
        class StdAllocator
        {
        public:
            using value_type = T;

            StdAllocator(const char* pName = nullptr) :
                mpAllocator(&GetFrameAllocator())
            {}
            StdAllocator(FrameAllocator& allocator) :
                mpAllocator(&allocator)
            {}
            template <typename U>
            StdAllocator(const StdAllocator<U>& other) noexcept :
                mpAllocator(other.mpAllocator)
            {}

            T* allocate(size_t n, int flags = 0)
            {
                return (T*)mpAllocator->Allocate(sizeof(T) * n);
            }

            T* allocate(size_t n, size_t alignment, size_t offset, int flags = 0)
            {
                return (T*)mpAllocator->AllocateAligned(sizeof(T) * n, alignment);
            }

            void deallocate(void* p, size_t n)
            {
                mpAllocator->FreeAligned(p);
            }

        private:
            template <typename U>
            friend class StdAllocator;

            FrameAllocator* mpAllocator;
        };

    public:
        RapidJsonAllocator& GetRapidJsonAllocator() { return mRapidJsonAllocator; }
        StdAllocator<char>& GetStdAllocator() { return mStdAllocator; }

    private:
        RapidJsonAllocator mRapidJsonAllocator;
        StdAllocator<char> mStdAllocator;
    };

    template <typename T1, typename T2>
    bool operator==(const FrameAllocator::StdAllocator<T1>& lhs, const FrameAllocator::StdAllocator<T2>& rhs) noexcept { return true; }
    template <typename T1, typename T2>
    bool operator!=(const FrameAllocator::StdAllocator<T1>& lhs, const FrameAllocator::StdAllocator<T2>& rhs) noexcept { return false; }

    // Define strings with frame allocator
    using FrameU8String = std::basic_string<U8Character, std::char_traits<U8Character>, FrameAllocator::StdAllocator<U8Character>>;
    using FrameU8StringRef = BaseStringRef<U8Character, FrameU8String>;

    using FrameU16String = std::basic_string<U16Character, std::char_traits<U16Character>, FrameAllocator::StdAllocator<U16Character>>;
    using FrameU16StringRef = BaseStringRef<U16Character, FrameU16String>;

    using FrameU32String = std::basic_string<U32Character, std::char_traits<U32Character>, FrameAllocator::StdAllocator<U32Character>>;
    using FrameU32StringRef = BaseStringRef<U32Character, FrameU32String>;

#if defined (DEFAULT_STRING_UTF8)
    using FrameString = FrameU8String;
    using FrameStringRef = FrameU8StringRef;
#elif defined (DEFAULT_STRING_UTF16)
    using FrameString = FrameU16String;
    using FrameStringRef = FrameU16StringRef;
#elif defined (DEFAULT_STRING_UTF32)
    using FrameString = FrameU32String;
    using FrameStringRef = FrameU32StringRef;
#endif

    // Define STL containers with frame allocator
    template <typename T>
    using FrameVector = std::vector<T, FrameAllocator::StdAllocator<T>>;

    template <typename Key, typename Value>
    using FrameMap = std::map<Key, Value, std::less<Key>, FrameAllocator::StdAllocator<std::pair<const Key, Value>>>;

    template <typename Key, typename Value>
    using FrameMultiMap = std::multimap<Key, Value, std::less<Key>, FrameAllocator::StdAllocator<std::pair<const Key, Value>>>;

    template <typename Key, typename Value>
    using FrameHashMap = std::unordered_map<Key, Value, std::hash<Key>, std::equal_to<Key>, FrameAllocator::StdAllocator<std::pair<const Key, Value>>>;

    // Define Json with frame allocator
    using FrameJson = rapidjson::GenericDocument<rapidjson::UTF8<>, FrameAllocator::RapidJsonAllocator>;
    using FrameJsonValue = rapidjson::GenericValue<rapidjson::UTF8<>, FrameAllocator::RapidJsonAllocator>;

    // Define formatting functions with frame allocator
    template <typename S, typename ...Args>
    inline std::basic_string<fmt::v6::char_t<S>, std::char_traits<fmt::v6::char_t<S>>, FrameAllocator::StdAllocator<fmt::v6::char_t<S>>>
        FrameFormat(const S& format_str, const Args& ...args)
    {
        return Format_CustomString<FrameAllocator::StdAllocator<fmt::v6::char_t<S>>>(format_str, args...);
    }
} // namespace cube
