#pragma once

#include "MetalHeader.h"

#include "MetalArgumentBufferManager.h"

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

        id<MTLDevice> GetMTLDevice() const { return mDevice; }

        MetalArgumentBufferManager& GetArgumentBufferManager() { return mArgumentBufferManager; }
        id<MTLCommandQueue> GetMainCommandQueue() const { return mMainCommandQueue; }

        void SetNumGPUSync(Uint32 newNumGPUSync);
        void BeginGPUFrame(Uint64 gpuFrame);
        void EndGPUFrame(Uint64 gpuFrame);
        void WaitAllGPUSync();

    private:
        id<MTLDevice> mDevice;

        Uint32 mNumGPUSync;
        id<MTLSharedEvent> mGPUSyncEvent;

        MetalArgumentBufferManager mArgumentBufferManager;
        id<MTLCommandQueue> mMainCommandQueue;
    };
} // namespace cube
