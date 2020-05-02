#include "Format.h"

namespace cube
{
    namespace format
    {
        namespace internal
        {
            template <typename T>
            T Align(T offset, T alignment)
            {
                return (offset + (alignment - 1)) & ~(alignment - 1);
            }

            thread_local FormatMemoryBlock thlFormatMemoryBlock;

            void* Allocate(size_t n)
            {
                Uint64 allocatedSize = (Uint64)thlFormatMemoryBlock.currentPtr - (Uint64)thlFormatMemoryBlock.startPtr;
                if(allocatedSize + n > blockSize)
                    return nullptr;

                void* ptr = thlFormatMemoryBlock.currentPtr;
                thlFormatMemoryBlock.currentPtr = (Uint8*)thlFormatMemoryBlock.currentPtr + n;

                return ptr;
            }

            void* Allocate(size_t n, size_t alignment)
            {
                Uint64 currentOffset = (Uint64)thlFormatMemoryBlock.currentPtr;

                Uint64 alignedOffset = Align(currentOffset, alignment);
                Uint8 alignGap = (Uint8)(alignedOffset - currentOffset);

                Uint64 allocatedSize = currentOffset - (Uint64)thlFormatMemoryBlock.startPtr;
                if(allocatedSize + alignGap + n > blockSize)
                    return nullptr;

                void* ptr = (void*)alignedOffset;
                thlFormatMemoryBlock.currentPtr = (Uint8*)thlFormatMemoryBlock.currentPtr + alignGap + n;

                return ptr;
            }

            void DiscardAllocations()
            {
                thlFormatMemoryBlock.currentPtr = thlFormatMemoryBlock.startPtr;
            }
        } // namespace internal
    } // namespace format
} // namespace cube
