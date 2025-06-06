#include "Allocator/FrameAllocator.h"

#include "Allocator/AllocatorUtility.h"
#include "Platform.h"

namespace cube
{
    thread_local FrameAllocator thlFrameAllocator;

    FrameAllocator& GetMyThreadFrameAllocator()
    {
        return thlFrameAllocator;
    }

    FrameAllocator::MemoryBlock::MemoryBlock(Uint64 size) :
        mSize(size)
    {
        if (mSize == 0) // Not initialized memory block
        {
            mStartPtr = mCurrentPtr = nullptr;
            return;
        }

        mStartPtr = platform::Platform::Allocate(size);

        mCurrentPtr = mStartPtr;
    }

    FrameAllocator::MemoryBlock::~MemoryBlock()
    {
        if (mSize == 0) // Not initialized memory block
        {
            return;
        }

        platform::Platform::Free(mStartPtr);
    }

    void* FrameAllocator::MemoryBlock::Allocate(Uint64 size)
    {
#ifdef CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
        // Uint64: allocated size
        size += sizeof(Uint64);
#endif

        Uint64 allocatedSize = (Uint64)mCurrentPtr - (Uint64)mStartPtr;
        if (allocatedSize + size > mSize)
            return nullptr;

        void* ptr = mCurrentPtr;
        mCurrentPtr = (Uint8*)mCurrentPtr + size;

#ifdef CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
        Uint64* storedSize = (Uint64*)ptr;
        *storedSize = size;

        return (Uint8*)ptr + sizeof(Uint64);
#else
        return ptr;
#endif
    }

    void* FrameAllocator::MemoryBlock::AllocateAligned(Uint64 size, Uint64 alignment)
    {
        // It doesn't store alignGap because the allocation isn't freed individually.
#ifdef CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
        // Uint64: allocated size
        size += sizeof(Uint64);
#endif

        Uint64 currentOffset = (Uint64)mCurrentPtr;
#ifdef CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
        //       |            |<------------------size------------------>|
        //       |<-alignGap->|<-sizeof(Uint64)->|                       |
        //       |            |                  |                       |
        // currentOffset                   alignedOffset

        Uint64 addedOffset = currentOffset + sizeof(Uint64);

        Uint64 alignedOffset = Align(addedOffset, alignment);
        Uint8 alignGap = (Uint8)(alignedOffset - addedOffset);
#else
        Uint64 alignedOffset = Align(currentOffset, alignment);
        Uint8 alignGap = (Uint8)(alignedOffset - currentOffset);
#endif

        Uint64 allocatedSize = currentOffset - (Uint64)mStartPtr;
        if (allocatedSize + alignGap + size > mSize)
            return nullptr;

        void* ptr = (void*)alignedOffset;
        mCurrentPtr = (Uint8*)mCurrentPtr + alignGap + size;

#ifdef CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
        Uint64* storedSize = (Uint64*)((Uint8*)ptr - sizeof(Uint64));
        *storedSize = alignGap + size;
#endif

        return ptr;
    }

    void FrameAllocator::MemoryBlock::DiscardAllocations()
    {
        mCurrentPtr = mStartPtr;
    }

    FrameAllocator::FrameAllocator() :
        mInitialized(false),
        mMemoryBlock(0),
        mStdAllocator(*this)
    {}

    FrameAllocator::~FrameAllocator()
    {}

    void FrameAllocator::Initialize(const char* debugName, Uint64 blockSize)
    {
#ifdef CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
        if (mInitialized)
        {
            CUBE_LOG(Warning, Allocator, "The frame allocator is already initialized. (name: {} / blockSize: {}) Skip the initialization.", debugName, blockSize);

            return;
        }
        mDebugName = debugName;
#endif
        mBlockSize = blockSize;
        mMemoryBlock = MemoryBlock(blockSize);

        mInitialized = true;
    }

    void FrameAllocator::Shutdown()
    {
#ifdef CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
        if (!mInitialized)
        {
            CUBE_LOG(Warning, Allocator, "The frame allocator is not initialized but try to shudown it.");

            return;
        }
#endif

        mAdditionalMemBlocks.clear();
        mMemoryBlock = MemoryBlock(0);

        mInitialized = false;
    }

    void* FrameAllocator::Allocate(Uint64 size)
    {
        void* ptr = mMemoryBlock.Allocate(size);

        if (ptr != nullptr)
        {
#ifdef CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
            mAllocatedSize += *(Uint64*)((Uint8*)ptr - sizeof(Uint64));
#endif
            return ptr;
        }

        // Try to allocate in additional blocks
        for (MemoryBlock& block : mAdditionalMemBlocks)
        {
            ptr = block.Allocate(size);

            if (ptr != nullptr)
            {
#ifdef CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
                mAllocatedSize += *(Uint64*)((Uint8*)ptr - sizeof(Uint64));
#endif
                return ptr;
            }
        }

        // Allocate new additional block
#ifdef CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
        AllocateAdditionalBlock(size + sizeof(Uint64));
#else
        AllocateAdditionalBlock(size);
#endif
        ptr = mAdditionalMemBlocks.back().Allocate(size);

#ifdef CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
        mAllocatedSize += *(Uint64*)((Uint8*)ptr - sizeof(Uint64));
#endif
        return ptr;
    }

    void FrameAllocator::Free(void* ptr)
    {
        // Do nothing except for debugging
#ifdef CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
        mAllocatedSize -= *(Uint64*)((Uint8*)ptr - sizeof(Uint64));
#endif
    }

    void* FrameAllocator::AllocateAligned(Uint64 size, Uint64 alignment)
    {
        void* ptr = mMemoryBlock.AllocateAligned(size, alignment);

        if (ptr != nullptr)
        {
#ifdef CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
            mAllocatedSize += *(Uint64*)((Uint8*)ptr - sizeof(Uint64));
#endif
            return ptr;
        }

        // Try to allocate in additional blocks
        for (MemoryBlock& block : mAdditionalMemBlocks)
        {
            ptr = block.AllocateAligned(size, alignment);

            if (ptr != nullptr)
            {
#ifdef CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
                mAllocatedSize += *(Uint64*)((Uint8*)ptr - sizeof(Uint64));
#endif
                return ptr;
            }
        }

        // Allocate new additional block
#ifdef CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
        AllocateAdditionalBlock(size + alignment + alignment + sizeof(Uint64));
#else
        AllocateAdditionalBlock(size + alignment);
#endif
        ptr = mAdditionalMemBlocks.back().AllocateAligned(size, alignment);

#ifdef CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
        mAllocatedSize += *(Uint64*)((Uint8*)ptr - sizeof(Uint64));
#endif
        return ptr;
    }

    void FrameAllocator::FreeAligned(void* ptr)
    {
        // Do nothing except for debugging
#ifdef CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
        mAllocatedSize -= *(Uint64*)((Uint8*)ptr - sizeof(Uint64));
#endif
    }

    void FrameAllocator::DiscardAllocations()
    {
#ifdef CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
        CHECK_FORMAT(mAllocatedSize == 0, "Not all allocations were freed in the frame allocator.");
#endif

        mMemoryBlock.DiscardAllocations();

        for (auto& block : mAdditionalMemBlocks)
        {
            block.DiscardAllocations();
        }
    }

    void FrameAllocator::AllocateAdditionalBlock(Uint64 size)
    {
        CUBE_LOG(Warning, Allocator, "Allocate additional block in FrameAllocation. It has performance cost. Try to increase default block size in FrameAllocator.");
        CUBE_LOG(Warning, Allocator, "Default size: {0} / Size to allocate: {1}", mBlockSize, size);

        mAdditionalMemBlocks.emplace_back(size);
    }
} // namespace cube
