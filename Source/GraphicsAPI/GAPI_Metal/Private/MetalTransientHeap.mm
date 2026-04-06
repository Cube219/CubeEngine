#include "MetalTransientHeap.h"

#include "Checker.h"
#include "Logger.h"
#include "MetalDevice.h"

namespace cube
{
    MetalTransientHeap::MetalTransientHeap(MetalDevice& device)
        : mDevice(device)
    {
    }

    MetalTransientHeap::~MetalTransientHeap()
    {
    }

    void MetalTransientHeap::Initialize(Uint32 numGPUSync)
    {
        MTLHeapDescriptor* desc = [MTLHeapDescriptor new];
        desc.storageMode = MTLStorageModePrivate;
        desc.size = DEFAULT_TRANSIENT_HEAP_SIZE;

        mHeap = [mDevice.GetMTLDevice() newHeapWithDescriptor:desc];
        CHECK(mHeap);
        [desc release];
    }

    void MetalTransientHeap::Shutdown()
    {
        [mHeap release];
    }

    void MetalTransientHeap::SetNumGPUSync(Uint32 newNumGPUSync)
    {
    }

    void MetalTransientHeap::MoveToNextIndex(Uint64 nextGPUFrame)
    {
        const Uint64 usedSize = [mHeap usedSize];
        CHECK_FORMAT(usedSize == 0, "Transient heap is not empty before move to next frame!");
    }

    void MetalTransientHeap::IncreaseHeapSize()
    {
        // TODO
    }
} // namespace cube
