#include "DeviceContextVk.h"

#include "../VulkanDevice.h"

namespace cube
{
    namespace rapi
    {
        DeviceContextVk::DeviceContextVk(VulkanDevice& device) :
            mDevice(device),
            mCommandPool(device, 0),
            mUploadCommandBuffer(device, mCommandPool)
        {
            mUploadCommandBuffer.Begin();
        }

        DeviceContextVk::~DeviceContextVk()
        {
        }
    } // namespace rapi
} // namespace cube
