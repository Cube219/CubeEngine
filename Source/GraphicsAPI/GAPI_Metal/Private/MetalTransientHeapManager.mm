#include "MetalTransientHeapManager.h"

#include "Allocator/AllocatorUtility.h"
#include "Checker.h"
#include "Logger.h"
#include "MetalDevice.h"

namespace cube
{
    MetalTransientHeapManager::MetalTransientHeapManager(MetalDevice& device)
        : mDevice(device)
    {
    }

    MetalTransientHeapManager::~MetalTransientHeapManager()
    {
    }

    void MetalTransientHeapManager::Initialize(Uint32 numGPUSync)
    {
        SetNumGPUSync(numGPUSync);
        mCurrentGPUFrame = 0;

        // Create initial size heap.
        CreateNewHeap(0);
    }

    void MetalTransientHeapManager::Shutdown()
    {
        for (TransientHeap& heap : mTransientHeaps)
        {
            heap.mtlHeap = nil;
        }
        mTransientHeaps.clear();
    }

    void MetalTransientHeapManager::SetNumGPUSync(Uint32 newNumGPUSync)
    {
        mNumGPUSync = newNumGPUSync;
    }

    void MetalTransientHeapManager::MoveToNextIndex(Uint64 nextGPUFrame)
    {
        mCurrentGPUFrame = nextGPUFrame;
        ClearUnusedTransientHeaps();
    }

    id<MTLHeap> MetalTransientHeapManager::GetMTLHeap(MTLSizeAndAlign sizeAndAlign)
    {
        TransientHeap* selectedHeap = nullptr;
        for (TransientHeap& heap : mTransientHeaps)
        {
            NSUInteger availableSize = [heap.mtlHeap maxAvailableSizeWithAlignment:sizeAndAlign.align];
            if (sizeAndAlign.size <= availableSize)
            {
                selectedHeap = &heap;
                break;
            }
        }
        if (!selectedHeap)
        {
            selectedHeap = CreateNewHeap(Align(sizeAndAlign.size, sizeAndAlign.align));
        }

        selectedHeap->lastUsedGPUFrame = mCurrentGPUFrame;
        return selectedHeap->mtlHeap;
    }

    MetalTransientHeapManager::TransientHeap* MetalTransientHeapManager::CreateNewHeap(Uint64 size)
    {
        const Uint64 newSize = std::max(size, DEFAULT_TRANSIENT_HEAP_SIZE);

        MTLHeapDescriptor* desc = [MTLHeapDescriptor new];
        desc.storageMode = MTLStorageModePrivate;
        desc.size = newSize;
        desc.hazardTrackingMode = MTLHazardTrackingModeTracked;

        id<MTLHeap> newHeap = [mDevice.GetMTLDevice() newHeapWithDescriptor:desc];
        CHECK(newHeap);

        mTransientHeaps.push_back({
            .mtlHeap = newHeap,
            .size = newSize,
            .lastUsedGPUFrame = mCurrentGPUFrame
        });

        return &mTransientHeaps.back();
    }

    void MetalTransientHeapManager::ClearUnusedTransientHeaps()
    {
        if (mCurrentGPUFrame < mNumGPUSync)
        {
            return;
        }

        for (int i = static_cast<int>(mTransientHeaps.size()) - 1; i >= 0; --i)
        {
            if (mTransientHeaps[i].lastUsedGPUFrame <= mCurrentGPUFrame - mNumGPUSync)
            {
                const Uint64 usedSize = [mTransientHeaps[i].mtlHeap usedSize];
                if (usedSize == 0)
                {
                    const int lastIndex = static_cast<int>(mTransientHeaps.size()) - 1;
                    if (lastIndex > i)
                    {
                        mTransientHeaps[i] = std::move(mTransientHeaps[lastIndex]);
                    }
                    mTransientHeaps.pop_back();
                }
            }
        }
    }
} // namespace cube
