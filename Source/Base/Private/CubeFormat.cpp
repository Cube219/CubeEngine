#include "Format.h"

#include <iostream>

namespace cube
{
    namespace format
    {
        namespace internal
        {
            constexpr Uint64 blockSize = 256 * 1024; // 256KiB
            struct FormatMemoryBlock
            {
                FormatMemoryBlock()
                {
                    startPtr = block;
                    currentPtr = startPtr;
                }

                char block[blockSize];
                void* startPtr;
                void* currentPtr;
            };

            thread_local FormatMemoryBlock thlFormatMemoryBlock;

            template <typename T>
            T Align(T offset, T alignment)
            {
                return (offset + (alignment - 1)) & ~(alignment - 1);
            }

            void* AllocateFormat(size_t n)
            {
                Uint64 allocatedSize = (Uint64)thlFormatMemoryBlock.currentPtr - (Uint64)thlFormatMemoryBlock.startPtr;
                if (allocatedSize + n > blockSize)
                {
                    std::wcout << L"CubeFormat: Failed to allocate memory from FormatAllocator. (blockSize: " << blockSize << " / allocatedSize: " << allocatedSize << " / sizeToAllocate: " << n << ")" << std::endl;
                    return nullptr;
                }

                void* ptr = thlFormatMemoryBlock.currentPtr;
                thlFormatMemoryBlock.currentPtr = (Uint8*)thlFormatMemoryBlock.currentPtr + n;

                return ptr;
            }

            void* AllocateFormat(size_t n, size_t alignment)
            {
                Uint64 currentOffset = (Uint64)thlFormatMemoryBlock.currentPtr;

                Uint64 alignedOffset = Align(currentOffset, alignment);
                Uint8 alignGap = (Uint8)(alignedOffset - currentOffset);

                Uint64 allocatedSize = currentOffset - (Uint64)thlFormatMemoryBlock.startPtr;
                if (allocatedSize + alignGap + n > blockSize)
                {
                    std::wcout << L"CubeFormat: Failed to allocate memory from FormatAllocator. (blockSize: " << blockSize << " / allocatedSize: " << allocatedSize << " / sizeToAllocate: " << alignGap + n << ")" << std::endl;
                    return nullptr;
                }

                void* ptr = (void*)alignedOffset;
                thlFormatMemoryBlock.currentPtr = (Uint8*)thlFormatMemoryBlock.currentPtr + alignGap + n;

                return ptr;
            }

            void DiscardFormatAllocations()
            {
                thlFormatMemoryBlock.currentPtr = thlFormatMemoryBlock.startPtr;
            }
        } // namespace internal
    } // namespace format
} // namespace cube
