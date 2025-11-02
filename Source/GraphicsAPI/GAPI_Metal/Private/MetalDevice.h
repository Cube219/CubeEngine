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

        void Initialize(id<MTLDevice> device);
        void Shutdown();

        bool CheckFeatureRequirements();

        id<MTLDevice> GetMTLDevice() const { return mDevice; }

        MetalArgumentBufferManager& GetArgumentBufferManager() { return mArgumentBufferManager; }

    private:
        id<MTLDevice> mDevice;

        MetalArgumentBufferManager mArgumentBufferManager;
    };
} // namespace cube
