#pragma once

#include "CoreHeader.h"

#include "CubeString.h"

namespace cube
{
    class FrameAllocator;

    FrameAllocator& GetMyThreadFrameAllocator();

    class FrameAllocator
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
        void AllocateAdditionalBlock(Uint64 size);

        bool mInitialized;
        Uint64 mBlockSize;
        MemoryBlock mMemoryBlock;

        Vector<MemoryBlock> mAdditionalMemBlocks;

#ifdef _DEBUG
        Uint64 mAllocatedSize = 0;
        const char* mDebugName;
#endif // _DEBUG

        // Class for Std
    public:
        template <typename T>
        class StdAllocator
        {
        public:
            using value_type = T;

            StdAllocator(const char* pName = nullptr) :
                mpAllocator(&GetMyThreadFrameAllocator())
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
        StdAllocator<char>& GetStdAllocator() { return mStdAllocator; }

    private:
        StdAllocator<char> mStdAllocator;
    };

    template <typename T1, typename T2>
    bool operator==(const FrameAllocator::StdAllocator<T1>& lhs, const FrameAllocator::StdAllocator<T2>& rhs) noexcept
    {
        return true;
    }
    template <typename T1, typename T2>
    bool operator!=(const FrameAllocator::StdAllocator<T1>& lhs, const FrameAllocator::StdAllocator<T2>& rhs) noexcept
    {
        return false;
    }

    // Define strings with frame allocator
    using FrameAnsiString = std::basic_string<AnsiCharacter, std::char_traits<AnsiCharacter>,
        FrameAllocator::StdAllocator<AnsiCharacter>>;

    using FrameU8String = std::basic_string<U8Character, std::char_traits<U8Character>, FrameAllocator::StdAllocator<U8Character>>;

    using FrameU16String = std::basic_string<U16Character, std::char_traits<U16Character>, FrameAllocator::StdAllocator<U16Character>>;

    using FrameU32String = std::basic_string<U32Character, std::char_traits<U32Character>, FrameAllocator::StdAllocator<U32Character>>;

#if defined(DEFAULT_STRING_UTF8)
    using FrameString = FrameU8String;
#elif defined(DEFAULT_STRING_UTF16)
    using FrameString = FrameU16String;
#elif defined(DEFAULT_STRING_UTF32)
    using FrameString = FrameU32String;
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
} // namespace cube
