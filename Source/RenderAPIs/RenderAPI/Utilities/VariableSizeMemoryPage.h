#pragma once

#include "Core/Allocator/AllocatorUtility.h"
#include "Core/Assertion.h"
#include "Utility/Types.h"

namespace cube
{
    namespace rapi
    {
        // Refer to VariableSizeAllocationsManager of DiligentEngine
        // https://github.com/DiligentGraphics/DiligentEngine (VariableSizeAllocationsManager.h)

        class VariableSizeMemoryPage
        {
        public:
            VariableSizeMemoryPage() {}
            VariableSizeMemoryPage(Uint64 size, Uint64 startOffset) :
                mStartOffset(startOffset), mMaxSize(size), mFreeSize(size)
            {
                InsertFreeBlock(startOffset, size);
            }
            ~VariableSizeMemoryPage() = default;

            VariableSizeMemoryPage(const VariableSizeMemoryPage& other) = delete;
            VariableSizeMemoryPage& operator=(const VariableSizeMemoryPage& rhs) = delete;

            VariableSizeMemoryPage(VariableSizeMemoryPage&& other) = default;
            VariableSizeMemoryPage& operator=(VariableSizeMemoryPage&& rhs) = default;

            struct Allocation
            {
                Uint64 offset;
                Uint64 unalignedOffset;
                Uint64 size;
            };

            Allocation Allocate(Uint64 size, Uint64 alignment)
            {
                auto smallestFreeBlockItIt = mFreeBlocksOrderBySize.lower_bound(size + alignment);
                if(smallestFreeBlockItIt == mFreeBlocksOrderBySize.end()) {
                    return Allocation{ Uint64InvalidValue, Uint64InvalidValue , Uint64InvalidValue };
                }
                auto smallestFreeBlockIt = smallestFreeBlockItIt->second;

                //     |<----------------------oldSize------ ..... ----------------->|
                //     |                               |<--- ..... ---newSize------->|
                //     |            |                  |     .....                   |
                //     |            |<------size------>|     .....                   |
                // oldOffset  alignedOffset       newOffset
                Uint64 oldFreeBlockOffset = smallestFreeBlockIt->first;
                Uint64 oldFreeBlockSize = smallestFreeBlockIt->second.size;

                Uint64 alignedOffset = Align(oldFreeBlockOffset, alignment);

                Uint64 newFreeBlockOffset = alignedOffset + size;
                Uint64 newFreeBlockSize = oldFreeBlockSize - (alignedOffset - oldFreeBlockOffset) - size;

                Uint64 unalignedOffset = oldFreeBlockOffset;

                RemoveFreeBlock(smallestFreeBlockIt);
                InsertFreeBlock(newFreeBlockOffset, newFreeBlockSize);

                mFreeSize -= size + (alignedOffset - oldFreeBlockOffset);

                return Allocation{ alignedOffset, unalignedOffset, size };
            }

            void Free(Allocation& alloc)
            {
                Uint64 freeOffset = alloc.unalignedOffset;
                Uint64 freeSize = alloc.size + (alloc.offset - alloc.unalignedOffset);

                auto nextBlockIter = mFreeBlocksOrderByOffset.upper_bound(freeOffset);
                Uint64 nextOffset;
                Uint64 nextSize;
                if(nextBlockIter != mFreeBlocksOrderByOffset.end()) {
                    nextOffset = nextBlockIter->first;
                    nextSize = nextBlockIter->second.size;
                } else {
                    nextOffset = mStartOffset;
                    nextSize = 0;
                }

                auto prevBlockIter = nextBlockIter;
                Uint64 prevOffset;
                Uint64 prevSize;
                if(prevBlockIter != mFreeBlocksOrderByOffset.begin()) {
                    --prevBlockIter;
                    prevOffset = prevBlockIter->first;
                    prevSize = prevBlockIter->second.size;
                } else {
                    prevBlockIter = mFreeBlocksOrderByOffset.end();
                    prevOffset = mStartOffset;
                    prevSize = 0;
                }

#ifdef _DEBUG
                // Check overlap between new and next
                if(nextBlockIter != mFreeBlocksOrderByOffset.end()) {
                    CHECK((freeOffset + freeSize) <= nextOffset, "Invalid free block insertion. New free block overlap next free block.");
                } else {
                    CHECK((freeOffset + freeSize) <= mMaxSize, "Invalid free block insertion. New free block exceeds max size.");
                }
                // Check overlap between prev and new
                if(prevBlockIter != mFreeBlocksOrderByOffset.end()) {
                    CHECK((prevOffset + prevSize) <= freeOffset, "Invalid free block insertion. New free block overlap previous free block.");
                } else {
                    CHECK(mStartOffset <= freeOffset, "Invalid free block insertion. New free block interrupt left boundary.");
                }
#endif // _DEBUG

                if(prevBlockIter != mFreeBlocksOrderByOffset.end() && prevOffset + prevSize == freeOffset) {
                    //     |                    |                  |
                    //     |<-----prevSize----->|<----freeSize---->|
                    //     |                    |                  |
                    // prevOffset           freeOffset

                    if(nextBlockIter != mFreeBlocksOrderByOffset.end() && freeOffset + freeSize == nextOffset) {
                        //     |                    |                  |                    |
                        //     |<-----prevSize----->|<----freeSize---->|<-----nextSize----->|
                        //     |                    |                  |                    |
                        // prevOffset           freeOffset         nextOffset
                        // -> Merge prev, free, next block
                        RemoveFreeBlock(nextBlockIter);
                        prevSize += freeSize + nextSize;
                        prevBlockIter->second.size = prevSize;
                    } else {
                        //     |                    |                  |       |                    |
                        //     |<-----prevSize----->|<----freeSize---->| ..... |<-----nextSize----->|
                        //     |                    |                  |       |                    |
                        // prevOffset           freeOffset                 nextOffset
                        // -> Merge prev, free
                        prevSize += freeSize;
                        prevBlockIter->second.size = prevSize;
                    }
                } else {
                    //     |                    |       |                  |
                    //     |<-----prevSize----->| ..... |<----freeSize---->|
                    //     |                    |       |                  |
                    // prevOffset                   freeOffset

                    if(nextBlockIter != mFreeBlocksOrderByOffset.end() && freeOffset + freeSize == nextOffset) {
                        //     |                    |       |                  |                    |
                        //     |<-----prevSize----->| ..... |<----freeSize---->|<-----nextSize----->|
                        //     |                    |       |                  |                    |
                        // prevOffset                   freeOffset          nextOffset
                        // -> Merge free, next
                        RemoveFreeBlock(nextBlockIter);
                        nextSize += freeSize;
                        nextOffset = freeOffset;
                        InsertFreeBlock(nextOffset, nextSize);
                    } else {
                        //     |                    |       |                  |       |                    |
                        //     |<-----prevSize----->| ..... |<----freeSize---->| ..... |<-----nextSize----->|
                        //     |                    |       |                  |       |                    |
                        // prevOffset                   freeOffset                 nextOffset
                        // -> Create new free block
                        InsertFreeBlock(freeOffset, freeSize);
                    }
                }

                mFreeSize += freeSize;
            }

            void Free(Allocation&& alloc)
            {
                Free(alloc);
            }

        private:
            struct FreeBlock;

            using OrderByOffsetMap = Map<Uint64, FreeBlock>;
            using OrderBySizeMultiMap = MultiMap<Uint64, OrderByOffsetMap::iterator>;

            struct FreeBlock
            {
                Uint64 size;
                OrderBySizeMultiMap::iterator orderBySizeIter;
                FreeBlock(Uint64 size) : size(size) {}
            };

            void InsertFreeBlock(Uint64 offset, Uint64 size)
            {
                auto newFreeBlockIterAndResult = mFreeBlocksOrderByOffset.emplace(offset, size);
                CHECK(newFreeBlockIterAndResult.second, "Failed to insert free block in mFreeBlocksOrderByOffset.");
                auto newFreeBlockIter = newFreeBlockIterAndResult.first;

                auto orderBySizeIter = mFreeBlocksOrderBySize.emplace(size, newFreeBlockIter);
                newFreeBlockIter->second.orderBySizeIter = orderBySizeIter;
            }

            void RemoveFreeBlock(OrderByOffsetMap::iterator iter)
            {
                mFreeBlocksOrderBySize.erase(iter->second.orderBySizeIter);
                mFreeBlocksOrderByOffset.erase(iter);
            }

            Uint64 mStartOffset;
            Uint64 mMaxSize;
            Uint64 mFreeSize;
            OrderByOffsetMap mFreeBlocksOrderByOffset;
            OrderBySizeMultiMap mFreeBlocksOrderBySize;
        };
    } // namespace rapi
} // namespace cube
