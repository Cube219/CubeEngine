#pragma once

#include "MetalHeader.h"

#include "MetalTimestampManager.h"
#include "MetalTransientHeapManager.h"
#include "MetalUploadManager.h"

namespace cube
{
    class MetalDevice
    {
    public:
        MetalDevice();
        ~MetalDevice();

        MetalDevice(const MetalDevice& other) = delete;
        MetalDevice& operator=(const MetalDevice& rhs) = delete;

        void Initialize(id<MTLDevice> device, Uint32 numGPUSync);
        void Shutdown();

        bool CheckFeatureRequirements();

        MetalTimestampManager& GetTimestampManager() { return mTimestampManager; }
        MetalTransientHeapManager& GetTransientHeapManager() { return mTransientHeapManager; }
        MetalUploadManager& GetUploadManager() { return mUploadManager; }

        id<MTLDevice> GetMTLDevice() const { return mDevice; }

        id<MTLCommandQueue> GetMainCommandQueue() const { return mMainCommandQueue; }

        void SetNumGPUSync(Uint32 newNumGPUSync);
        void BeginGPUFrame(Uint64 gpuFrame);
        void EndGPUFrame(Uint64 gpuFrame);
        void WaitAllGPUSync();

    private:
        id<MTLDevice> mDevice;

        MetalTimestampManager mTimestampManager;
        MetalTransientHeapManager mTransientHeapManager;
        MetalUploadManager mUploadManager;

        Uint32 mNumGPUSync;
        id<MTLSharedEvent> mGPUSyncEvent;

        id<MTLCommandQueue> mMainCommandQueue;
    };
} // namespace cube
