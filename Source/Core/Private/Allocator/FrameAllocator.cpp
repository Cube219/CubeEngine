#include "Allocator/FrameAllocator.h"

#include <algorithm>

#include "Allocator/AllocatorUtility.h"

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

        mStartPtr = malloc(size);

        mCurrentPtr = mStartPtr;
    }

    FrameAllocator::MemoryBlock::~MemoryBlock()
    {
        if (mSize == 0) // Not initialized memory block
        {
            return;
        }

        free(mStartPtr);
    }

    void* FrameAllocator::MemoryBlock::Allocate(Uint64 size)
    {
#if CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
        // Uint64: allocated size
        size += sizeof(Uint64);
#endif

        Uint64 allocatedSize = (Uint64)mCurrentPtr - (Uint64)mStartPtr;
        if (allocatedSize + size > mSize)
            return nullptr;

        void* ptr = mCurrentPtr;
        mCurrentPtr = (Uint8*)mCurrentPtr + size;

#if CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
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
#if CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
        // Uint64: allocated size
        size += sizeof(Uint64);
#endif

        Uint64 currentOffset = (Uint64)mCurrentPtr;
#if CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
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

#if CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
        Uint64* storedSize = (Uint64*)((Uint8*)ptr - sizeof(Uint64));
        *storedSize = alignGap + size;
#endif

        return ptr;
    }

    void FrameAllocator::MemoryBlock::DiscardAllocations()
    {
        mCurrentPtr = mStartPtr;
    }

    bool FrameAllocator::MemoryBlock::Contains(void* ptr) const
    {
        return (Uint64)mStartPtr <= (Uint64)ptr && (Uint64)ptr <= (Uint64)mStartPtr + mSize;
    }

    FrameAllocator::FrameAllocator() :
        mInitialized(false),
        mMemoryBlock(0),
        mStdAllocator(*this)
    {}

    FrameAllocator::~FrameAllocator()
    {
        if (mInitialized)
        {
            Shutdown();
        }
    }

    void FrameAllocator::Initialize(const char* debugName, Uint64 blockSize)
    {
#if CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
        if (mInitialized)
        {
            CUBE_LOG(Warning, Allocator, "The frame allocator is already initialized. (name: {0} / blockSize: {1}) Skip the initialization.", debugName, blockSize);

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
#if CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
        if (!mInitialized)
        {
            CUBE_LOG(Warning, Allocator, "The frame allocator is not initialized but shutdown was triggered.");

            return;
        }
#endif

        mAdditionalMemBlocks.clear();
        mMemoryBlock = MemoryBlock(0);

        mInitialized = false;
    }

    void* FrameAllocator::Allocate(Uint64 size)
    {
        EnsureInitialization();

        void* ptr = mMemoryBlock.Allocate(size);

        if (ptr != nullptr)
        {
#if CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
            mAllocatedSize += *(Uint64*)((Uint8*)ptr - sizeof(Uint64));
            AddAllocationInfo(ptr);
#endif
            return ptr;
        }

        // Try to allocate in additional blocks
        for (MemoryBlock& block : mAdditionalMemBlocks)
        {
            ptr = block.Allocate(size);

            if (ptr != nullptr)
            {
#if CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
                mAllocatedSize += *(Uint64*)((Uint8*)ptr - sizeof(Uint64));
                AddAllocationInfo(ptr);
#endif
                return ptr;
            }
        }

        // Allocate new additional block
#if CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
        AllocateAdditionalBlock(size + sizeof(Uint64));
#else
        AllocateAdditionalBlock(size);
#endif
        ptr = mAdditionalMemBlocks.back().Allocate(size);

#if CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
        mAllocatedSize += *(Uint64*)((Uint8*)ptr - sizeof(Uint64));
        AddAllocationInfo(ptr);
#endif

        // Log after the allocation is completed because the logger also allocates in the frame allocator.
        LogAdditionalBlockAllocation(size);

        return ptr;
    }

    void FrameAllocator::Free(void* ptr)
    {
        // Do nothing except for tracking
#if CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
        EnsureInitialization();

        if (ValidateAndRemoveAllocationInfo(ptr))
        {
            mAllocatedSize -= *(Uint64*)((Uint8*)ptr - sizeof(Uint64));
        }
#endif
    }

    void* FrameAllocator::AllocateAligned(Uint64 size, Uint64 alignment)
    {
        EnsureInitialization();

        void* ptr = mMemoryBlock.AllocateAligned(size, alignment);

        if (ptr != nullptr)
        {
#if CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
            mAllocatedSize += *(Uint64*)((Uint8*)ptr - sizeof(Uint64));
            AddAllocationInfo(ptr);
#endif
            return ptr;
        }

        // Try to allocate in additional blocks
        for (MemoryBlock& block : mAdditionalMemBlocks)
        {
            ptr = block.AllocateAligned(size, alignment);

            if (ptr != nullptr)
            {
#if CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
                mAllocatedSize += *(Uint64*)((Uint8*)ptr - sizeof(Uint64));
                AddAllocationInfo(ptr);
#endif
                return ptr;
            }
        }

        // Allocate new additional block
#if CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
        AllocateAdditionalBlock(size + alignment + alignment + sizeof(Uint64));
#else
        AllocateAdditionalBlock(size + alignment);
#endif
        ptr = mAdditionalMemBlocks.back().AllocateAligned(size, alignment);

#if CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
        mAllocatedSize += *(Uint64*)((Uint8*)ptr - sizeof(Uint64));
        AddAllocationInfo(ptr);
#endif

        // Log after the allocation is completed because the logger also allocates in the frame allocator.
        LogAdditionalBlockAllocation(size);

        return ptr;
    }

    void FrameAllocator::FreeAligned(void* ptr)
    {
        // Do nothing except for tracking
#if CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
        EnsureInitialization();

        if (ValidateAndRemoveAllocationInfo(ptr))
        {
            mAllocatedSize -= *(Uint64*)((Uint8*)ptr - sizeof(Uint64));
        }
#endif
    }

    void FrameAllocator::DiscardAllocations()
    {
        if (!mInitialized)
        {
            return;
        }

        CHECK_FORMAT(mCurrentAllocationScopeSnapshots.empty(), "Discard allocations while the allocation scope is active.");

#if CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
        CHECK_FORMAT(mAllocatedSize == 0, "Not all allocations were freed in the frame allocator.");
#endif

        mMemoryBlock.DiscardAllocations();

        for (auto& block : mAdditionalMemBlocks)
        {
            block.DiscardAllocations();
        }
    }

    void FrameAllocator::PushAllocationScope(FrameAllocationScope& scope)
    {
        EnsureInitialization();

        AllocationScopeSnapshot snapshot;
        snapshot.scope = &scope;
        snapshot.memoryBlockCurrentPtr = mMemoryBlock.mCurrentPtr;
        for (const MemoryBlock& additionalMemoryBlock : mAdditionalMemBlocks)
        {
            snapshot.additionalMemoryBlockCurrentPtrs.push_back(additionalMemoryBlock.mCurrentPtr);
        }
        mCurrentAllocationScopeSnapshots.emplace_back(std::move(snapshot));
    }

    void FrameAllocator::PopAllocationScope(FrameAllocationScope& scope)
    {
        // Do not hold the reference of the snapshot while checking.
        // Failed check writes a log and the logger pushes another allocation scope. (It can reallocate the snapshot vector.)
        CHECK(mCurrentAllocationScopeSnapshots.size() > 0);
        CHECK_FORMAT(&scope == mCurrentAllocationScopeSnapshots.back().scope,
            "Allocation scopes must be popped in reverse order. (Scope: {0}:{1})",
            scope.GetLocation().file_name(), scope.GetLocation().line());

#if CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
        CHECK_FORMAT(mCurrentAllocationScopeSnapshots.back().currentScopedAllocationPtrs.empty(),
            "Not all allocations in the allocation scope were freed. (Scope: {0}:{1} / Remaining allocations: {2})",
            scope.GetLocation().file_name(), scope.GetLocation().line(), mCurrentAllocationScopeSnapshots.back().currentScopedAllocationPtrs.size());

        // Remaining allocations are rolled back with the scope, so exclude them from the allocated size.
        // Freeing them later is reported as an invalid free (if detected) and doesn't change the allocated size.
        for (void* remainingPtr : mCurrentAllocationScopeSnapshots.back().currentScopedAllocationPtrs)
        {
            mAllocatedSize -= *(Uint64*)((Uint8*)remainingPtr - sizeof(Uint64));
        }
#endif

        const AllocationScopeSnapshot& lastSnapshot = mCurrentAllocationScopeSnapshots.back();

        mMemoryBlock.mCurrentPtr = lastSnapshot.memoryBlockCurrentPtr;
        for (int i = 0; i < mAdditionalMemBlocks.size(); ++i)
        {
            if (i < lastSnapshot.additionalMemoryBlockCurrentPtrs.size())
            {
                mAdditionalMemBlocks[i].mCurrentPtr = lastSnapshot.additionalMemoryBlockCurrentPtrs[i];
            }
            else
            {
                mAdditionalMemBlocks[i].DiscardAllocations();
            }
        }

        mCurrentAllocationScopeSnapshots.pop_back();
    }

    void FrameAllocator::EnsureInitialization()
    {
        if (!mInitialized)
        {
            Initialize("FrameAllocator (auto initialized)");

            CUBE_LOG(Warning, Allocator, "Frame allocator was used before initialized. Auto-initialized with default settings.");
        }
    }

    void FrameAllocator::AllocateAdditionalBlock(Uint64 size)
    {
        // Allocate additional margin so that following allocations (format, log, ...) can use.
        mAdditionalMemBlocks.emplace_back(size + 10u * 1024 * 1024);
    }

    void FrameAllocator::LogAdditionalBlockAllocation(Uint64 size)
    {
        CUBE_LOG(Warning, Allocator, "Allocate additional block in FrameAllocation. It has performance cost. Try to increase default block size in FrameAllocator.");
        CUBE_LOG(Warning, Allocator, "Default size: {0} / Size to allocate: {1}", mBlockSize, size);
    }

#if CUBE_FRAME_ALLOCATOR_TRACK_ALLOCATION
    void FrameAllocator::AddAllocationInfo(void* ptr)
    {
        if (!mCurrentAllocationScopeSnapshots.empty())
        {
            AllocationScopeSnapshot& lastSnapshot = mCurrentAllocationScopeSnapshots.back();

            lastSnapshot.currentScopedAllocationPtrs.insert(ptr);
        }
    }

    bool FrameAllocator::ValidateAndRemoveAllocationInfo(void* ptr)
    {
        const MemoryBlock* block = nullptr;
        Uint64 additionalBlockIndex = 0;
        if (mMemoryBlock.Contains(ptr))
        {
            block = &mMemoryBlock;
        }
        else
        {
            for (Uint64 i = 0; i < mAdditionalMemBlocks.size(); ++i)
            {
                if (mAdditionalMemBlocks[i].Contains(ptr))
                {
                    block = &mAdditionalMemBlocks[i];
                    additionalBlockIndex = i;
                    break;
                }
            }
        }
        const bool isInAllocator = (block != nullptr);
        CHECK_FORMAT(isInAllocator, "Freed pointer was not allocated by this frame allocator. (Allocator: {0} / Pointer: {1})", mDebugName, ptr);
        if (!isInAllocator)
        {
            return false;
        }

        const bool isRolledBack = (Uint64)ptr > (Uint64)block->mCurrentPtr;
        CHECK_FORMAT(!isRolledBack, "Freed allocation was already rolled back or discarded.��(Allocator: {0} / Pointer: {1})", mDebugName, ptr);
        if (isRolledBack)
        {
            return false;
        }

        // Allocations after the outermost scope snapshot are always tracked by one of the active scopes.
        // (Allocations of popped scopes are rolled back with their tracking info.)
        bool isInScopedRegion = false;
        if (!mCurrentAllocationScopeSnapshots.empty())
        {
            const AllocationScopeSnapshot& outermostSnapshot = mCurrentAllocationScopeSnapshots.front();
            if (block == &mMemoryBlock)
            {
                isInScopedRegion = (Uint64)ptr > (Uint64)outermostSnapshot.memoryBlockCurrentPtr;
            }
            else if (additionalBlockIndex < outermostSnapshot.additionalMemoryBlockCurrentPtrs.size())
            {
                isInScopedRegion = (Uint64)ptr > (Uint64)outermostSnapshot.additionalMemoryBlockCurrentPtrs[additionalBlockIndex];
            }
            else // The block was added after the outermost scope was pushed.
            {
                isInScopedRegion = true;
            }
        }

        for (auto it = mCurrentAllocationScopeSnapshots.rbegin(); it != mCurrentAllocationScopeSnapshots.rend(); ++it)
        {
            auto findIt = it->currentScopedAllocationPtrs.find(ptr);
            if (findIt != it->currentScopedAllocationPtrs.end())
            {
                it->currentScopedAllocationPtrs.erase(findIt);
                return true;
            }
        }

        CHECK_FORMAT(!isInScopedRegion, "Freed allocation is not tracked by any active allocation scope. It may be freed twice or have escaped from a popped allocation scope. (Allocator: {0} / Pointer: {1})", mDebugName, ptr);

        return !isInScopedRegion;
    }
#endif

    FrameAllocationScope::FrameAllocationScope(std::source_location location)
        : FrameAllocationScope(GetMyThreadFrameAllocator(), location)
    {}

    FrameAllocationScope::FrameAllocationScope(FrameAllocator& allocator, std::source_location location)
        : mAllocator(allocator)
        , mLocation(location)
    {
        mAllocator.PushAllocationScope(*this);
    }

    FrameAllocationScope::~FrameAllocationScope()
    {
        mAllocator.PopAllocationScope(*this);
    }
} // namespace cube
