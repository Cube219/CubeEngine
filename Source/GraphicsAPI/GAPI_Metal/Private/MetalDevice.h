#pragma once

#include "MetalHeader.h"

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

        id<MTLDevice> GetDevice() const { return mDevice; }

    private:
        id<MTLDevice> mDevice;
    };
} // namespace cube
