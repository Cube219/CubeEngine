#pragma once

#include "MetalHeader.h"

namespace cube
{
    class MetalDevice;

    class MetalTransientHeap
    {
    public:
        static constexpr int DEFAULT_TRANSIENT_HEAP_SIZE = 100 * 1024 * 1024; // 100 MiB

    public:
        MetalTransientHeap(MetalDevice& device);
        ~MetalTransientHeap();

        void Initialize(Uint32 numGPUSync);
        void Shutdown();

        void SetNumGPUSync(Uint32 newNumGPUSync);
        void MoveToNextIndex(Uint64 nextGPUFrame);

        id<MTLHeap> GetMTLHeap() const { return mHeap; }
        void IncreaseHeapSize();

    private:
        MetalDevice& mDevice;

        id<MTLHeap> mHeap;
    };
} // namespace cube
