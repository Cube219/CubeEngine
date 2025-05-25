#include "MetalDevice.h"

namespace cube
{
    MetalDevice::MetalDevice()
    {
    }

    MetalDevice::~MetalDevice()
    {
    }

    void MetalDevice::Initialize(id<MTLDevice> device)
    {
        mDevice = device;
    }

    void MetalDevice::Shutdown()
    {
    }
} // namespace cube
