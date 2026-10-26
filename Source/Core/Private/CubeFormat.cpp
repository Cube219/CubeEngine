#include "Format.h"

namespace cube
{
    namespace format
    {
        namespace internal
        {
            thread_local FrameAllocator thlFormatScratchAllocator;

            FrameAllocator& GetMyThreadFormatScratchAllocator()
            {
                if (!thlFormatScratchAllocator.IsInitialized())
                {
                    thlFormatScratchAllocator.Initialize("Format scratch allocator", 256 * 1024); // 256 KiB
                }

                return thlFormatScratchAllocator;
            }
        } // namespace internal
    } // namespace format
} // namespace cube
