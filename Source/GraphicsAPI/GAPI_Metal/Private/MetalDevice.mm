#include "MetalDevice.h"

#include <Metal/Metal.h>

#include "Logger.h"
#include "MacOS/MacOSString.h"

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

    bool MetalDevice::CheckFeatureRequirements()
    {
        // Argument Buffer Tier2 (Unsized array of descriptors is needed in bindless)
        if (mDevice.argumentBuffersSupport < MTLArgumentBuffersTier2)
        {
            const int tierValueOffset = 1 - MTLArgumentBuffersTier1;
            CUBE_LOG(Info, Metal, "Device {0} does not support Argument Buffer Tier 2 (Maximum: Tier{1}), which is required.", mDevice.name, (int)(mDevice.argumentBuffersSupport) + tierValueOffset);
            return false;
        }

        return true;
    }
} // namespace cube
