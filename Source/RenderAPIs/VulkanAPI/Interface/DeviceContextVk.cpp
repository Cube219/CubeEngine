#include "DeviceContextVk.h"

#include "../VulkanDevice.h"

namespace cube
{
    namespace rapi
    {
        DeviceContextVk::DeviceContextVk(VulkanDevice& device) :
            mDevice(device),
            mGraphicsCommandPool(device),
            mComputeCommandPool(device)
        {
            auto& queueManager = device.GetQueueManager();

            mGraphicsCommandPool.CreatePool(VulkanCommandBufferType::Graphics);
            mComputeCommandPool.CreatePool(VulkanCommandBufferType::Compute);
        }

        DeviceContextVk::~DeviceContextVk()
        {
            mComputeCommandPool.DestroyPool();
            mGraphicsCommandPool.DestroyPool();
        }
    } // namespace rapi
} // namespace cube
