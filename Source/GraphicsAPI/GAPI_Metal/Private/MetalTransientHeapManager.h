#pragma once

#include "MetalHeader.h"

namespace cube
{
    class MetalDevice;

    class MetalTransientHeapManager
    {
    public:
        static constexpr Uint64 DEFAULT_TRANSIENT_HEAP_SIZE = 128 * 1024 * 1024; // 128 MiB

    public:
        MetalTransientHeapManager(MetalDevice& device);
        ~MetalTransientHeapManager();

        void Initialize(Uint32 numGPUSync);
        void Shutdown();

        void SetNumGPUSync(Uint32 newNumGPUSync);
        void MoveToNextIndex(Uint64 nextGPUFrame);

        id<MTLHeap> GetMTLHeap(MTLSizeAndAlign sizeAndAlign);

    private:
        MetalDevice& mDevice;

        Uint32 mNumGPUSync;
        Uint64 mCurrentGPUFrame;

        struct TransientHeap
        {
            id<MTLHeap> mtlHeap;
            Uint64 size;
            Uint64 lastUsedGPUFrame;
        };
        TransientHeap* CreateNewHeap(Uint64 size);
        void ClearUnusedTransientHeaps();

        Vector<TransientHeap> mTransientHeaps;
    };
} // namespace cube
