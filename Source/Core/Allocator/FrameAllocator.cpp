#include "FrameAllocator.h"

#include "Platform/Platform.h"
#include "AllocatorUtility.h"
#include "../LogWriter.h"

namespace cube
{
    thread_local FrameAllocator thlFrameAllocator;

    FrameAllocator& GetFrameAllocator()
    {
        return thlFrameAllocator;
    }

    /////////////////////////////////
    // FrameAllocator::MemoryBlock //
    /////////////////////////////////

    FrameAllocator::MemoryBlock::MemoryBlock(Uint64 size) :
        mSize(size)
    {
        if(mSize == 0) { // Not initialized memory block
            mStartPtr = mCurrentPtr = nullptr;
            return;
        }

        mStartPtr = platform::Platform::Allocate(size);

        mCurrentPtr = mStartPtr;
    }

    FrameAllocator::MemoryBlock::~MemoryBlock()
    {
        if(mSize == 0) // Not initialized memory block
            return;

        platform::Platform::Free(mStartPtr);
    }

    void* FrameAllocator::MemoryBlock::Allocate(Uint64 size)
    {
#ifdef _DEBUG
        // Uint64: allocated size
        size += sizeof(Uint64);
#endif // _DEBUG

        Uint64 allocatedSize = (Uint64)mCurrentPtr - (Uint64)mStartPtr;
        if(allocatedSize + size > mSize)
            return nullptr;

        void* ptr = mCurrentPtr;
        mCurrentPtr = (Uint8*)mCurrentPtr + size;

#ifdef _DEBUG
        Uint64* storedSize = (Uint64*)ptr;
        *storedSize = size;

        return (Uint8*)ptr + sizeof(Uint64);
#else
        return ptr;
#endif // _DEBUG
    }

    void* FrameAllocator::MemoryBlock::AllocateAligned(Uint64 size, Uint64 alignment)
    {
        // It doesn't store alignGap because the allocation isn't freed individually.
#ifdef _DEBUG
        // Uint64: allocated size
        size += sizeof(Uint64);
#endif // _DEBUG

        Uint64 currentOffset = (Uint64)mCurrentPtr;
#ifdef _DEBUG
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
#endif // _DEBUG

        Uint64 allocatedSize = currentOffset - (Uint64)mStartPtr;
        if(allocatedSize + alignGap + size > mSize)
            return nullptr;

        void* ptr = (void*)alignedOffset;
        mCurrentPtr = (Uint8*)mCurrentPtr + alignGap + size;

#ifdef _DEBUG
        Uint64* storedSize = (Uint64*)((Uint8*)ptr - sizeof(Uint64));
        *storedSize = alignGap + size;
#endif // _DEBUG

        return ptr;
    }

    void FrameAllocator::MemoryBlock::DiscardAllocations()
    {
        mCurrentPtr = mStartPtr;
    }

    ////////////////////
    // FrameAllocator //
    ////////////////////

    FrameAllocator::FrameAllocator() :
        mMemoryBlock(0),
        mRapidJsonAllocator(*this), mStdAllocator(*this)
    {}

    FrameAllocator::~FrameAllocator()
    {}

    void FrameAllocator::Initialize(const char* debugName, Uint64 blockSize)
    {
#ifdef _DEBUG
        mDebugName = debugName;
#endif // _DEBUG
        mBlockSize = blockSize;
        mMemoryBlock = MemoryBlock(blockSize);
    }

    void FrameAllocator::Shutdown()
    {
        mAdditionalMemBlocks.clear();
        mMemoryBlock = MemoryBlock(0);
    }

    void* FrameAllocator::Allocate(Uint64 size)
    {
        void* ptr = mMemoryBlock.Allocate(size);

        if(ptr != nullptr) {
#ifdef _DEBUG
            mAllocatedSize += *(Uint64*)((Uint8*)ptr - sizeof(Uint64));
#endif // _DEBUG
            return ptr;
        }

        // Try to allocate in additional blocks
        for(auto& block : mAdditionalMemBlocks) {
            ptr = block.Allocate(size);

            if(ptr != nullptr) {
#ifdef _DEBUG
                mAllocatedSize += *(Uint64*)((Uint8*)ptr - sizeof(Uint64));
#endif // _DEBUG
                return ptr;
            }
        }

        // Allocate new additional block
        AllocateAdditioanlBlock(size);
        ptr = mAdditionalMemBlocks.back().Allocate(size);

#ifdef _DEBUG
        mAllocatedSize += *(Uint64*)((Uint8*)ptr - sizeof(Uint64));
#endif // _DEBUG
        return ptr;
    }

    void FrameAllocator::Free(void* ptr)
    {
        // Do nothing except for debugging
#ifdef _DEBUG
        mAllocatedSize -= *(Uint64*)((Uint8*)ptr - sizeof(Uint64));
#endif // _DEBUG
    }

    void* FrameAllocator::AllocateAligned(Uint64 size, Uint64 alignment)
    {
        void* ptr = mMemoryBlock.AllocateAligned(size, alignment);

        if(ptr != nullptr) {
#ifdef _DEBUG
            mAllocatedSize += *(Uint64*)((Uint8*)ptr - sizeof(Uint64));
#endif // _DEBUG
            return ptr;
        }

        // Try to allocate in additional blocks
        for(auto& block : mAdditionalMemBlocks) {
            ptr = block.AllocateAligned(size, alignment);

            if(ptr != nullptr) {
#ifdef _DEBUG
                mAllocatedSize += *(Uint64*)((Uint8*)ptr - sizeof(Uint64));
#endif // _DEBUG
                return ptr;
            }
        }

        // Allocate new additional block
        AllocateAdditioanlBlock(size + alignment);
        ptr = mAdditionalMemBlocks.back().AllocateAligned(size, alignment);

#ifdef _DEBUG
        mAllocatedSize += *(Uint64*)((Uint8*)ptr - sizeof(Uint64));
#endif // _DEBUG
        return ptr;
    }

    void FrameAllocator::FreeAligned(void* ptr)
    {
        // Do nothing except for debugging
#ifdef _DEBUG
        mAllocatedSize -= *(Uint64*)((Uint8*)ptr - sizeof(Uint64));
#endif // _DEBUG
    }

    void FrameAllocator::DiscardAllocations()
    {
#ifdef _DEBUG
        CHECK(mAllocatedSize == 0, "Not all allocations were freed in the frame allocator.");
#endif // _DEBUG

        mMemoryBlock.DiscardAllocations();

        for(auto& block : mAdditionalMemBlocks) {
            block.DiscardAllocations();
        }
    }

    void FrameAllocator::AllocateAdditioanlBlock(Uint64 size)
    {
        CUBE_LOG(LogType::Warning, "Allocate additional block in FrameAllocation. It has performance cost. Try to increase default block size in FrameAllocator.");
        CUBE_LOG(LogType::Warning, "Default size: {0} / Size to allocate: {1}", mBlockSize, size);

        mAdditionalMemBlocks.emplace_back(size);
    }
} // namespace cube
