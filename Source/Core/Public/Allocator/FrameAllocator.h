#pragma once

#include <source_location>

#include "CoreHeader.h"

#include "CubeString.h"

#ifndef CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
#define CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION CUBE_DEBUG
#endif

namespace cube
{
    class FrameAllocator;
    class FrameAllocationScope;

    CUBE_CORE_EXPORT FrameAllocator& GetMyThreadFrameAllocator();

    class CUBE_CORE_EXPORT FrameAllocator
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

            bool Contains(void* ptr) const;

        private:
            friend class FrameAllocator;

            Uint64 mSize;

            void* mStartPtr;
            void* mCurrentPtr;
        };

    public:
        FrameAllocator();
        ~FrameAllocator();

        FrameAllocator(const FrameAllocator& other) = delete;
        FrameAllocator& operator=(const FrameAllocator& rhs) = delete;

        void Initialize(const char* debugName, Uint64 blockSize = 10 * 1024 * 1024); // 10 MiB
        void Shutdown();

        bool IsInitialized() const { return mInitialized; }

        void* Allocate(Uint64 size);
        void Free(void* ptr);

        void* AllocateAligned(Uint64 size, Uint64 alignment);
        void FreeAligned(void* ptr);

        void DiscardAllocations();

        void PushAllocationScope(FrameAllocationScope& scope);
        void PopAllocationScope(FrameAllocationScope& scope);

    private:
        void EnsureInitialization();
        void AllocateAdditionalBlock(Uint64 size);
        void LogAdditionalBlockAllocation(Uint64 size);

#if CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
        void AddAllocationInfo(void* ptr);
        bool ValidateAndRemoveAllocationInfo(void* ptr);
#endif

        bool mInitialized;
        Uint64 mBlockSize;
        MemoryBlock mMemoryBlock;

        Vector<MemoryBlock> mAdditionalMemBlocks;

        struct AllocationScopeSnapshot
        {
            FrameAllocationScope* scope;
            void* memoryBlockCurrentPtr;
            Vector<void*> additionalMemoryBlockCurrentPtrs;

#if CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
            Set<void*> currentScopedAllocationPtrs;
#endif
        };
        Vector<AllocationScopeSnapshot> mCurrentAllocationScopeSnapshots;

#if CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
        Uint64 mAllocatedSize = 0;
        const char* mDebugName;
#endif

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
                return (T*)mpAllocator->AllocateAligned(sizeof(T) * n, alignof(T));
            }

            void deallocate(void* p, size_t n)
            {
                mpAllocator->FreeAligned(p);
            }

            FrameAllocator* GetFrameAllocator() const { return mpAllocator; }

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
        return lhs.GetFrameAllocator() == rhs.GetFrameAllocator();
    }
    template <typename T1, typename T2>
    bool operator!=(const FrameAllocator::StdAllocator<T1>& lhs, const FrameAllocator::StdAllocator<T2>& rhs) noexcept
    {
        return !(lhs == rhs);
    }

    class CUBE_CORE_EXPORT FrameAllocationScope
    {
    public:
        FrameAllocationScope(std::source_location location = std::source_location::current());
        FrameAllocationScope(FrameAllocator& allocator, std::source_location location = std::source_location::current());
        ~FrameAllocationScope();

        FrameAllocationScope(const FrameAllocationScope& other) = delete;
        FrameAllocationScope& operator=(const FrameAllocationScope& rhs) = delete;

        const std::source_location& GetLocation() const { return mLocation; }

    private:
        FrameAllocator& mAllocator;
        std::source_location mLocation;
    };

    // Define strings with frame allocator
    template <typename Char>
    using TFrameString = std::basic_string<Char, std::char_traits<Char>, FrameAllocator::StdAllocator<Char>>;

    using FrameAnsiString = TFrameString<AnsiCharacter>;
    using FrameU8String = TFrameString<U8Character>;
    using FrameU16String = TFrameString<U16Character>;
    using FrameU32String = TFrameString<U32Character>;

#if defined(CUBE_DEFAULT_STRING_UTF8)
    using FrameString = FrameU8String;
#elif defined(CUBE_DEFAULT_STRING_UTF16)
    using FrameString = FrameU16String;
#elif defined(CUBE_DEFAULT_STRING_UTF32)
    using FrameString = FrameU32String;
#endif

    // Define STL containers with frame allocator
    template <typename T>
    using FrameVector = std::vector<T, FrameAllocator::StdAllocator<T>>;

    template <typename Key>
    using FrameSet = std::set<Key, std::less<Key>, FrameAllocator::StdAllocator<Key>>;

    template <typename Key, typename Value>
    using FrameMap = std::map<Key, Value, std::less<Key>, FrameAllocator::StdAllocator<std::pair<const Key, Value>>>;

    template <typename Key, typename Value>
    using FrameMultiMap = std::multimap<Key, Value, std::less<Key>, FrameAllocator::StdAllocator<std::pair<const Key, Value>>>;

    template <typename Key, typename Value, typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key>>
    using FrameHashMap = std::unordered_map<Key, Value, Hash, KeyEqual, FrameAllocator::StdAllocator<std::pair<const Key, Value>>>;
} // namespace cube
