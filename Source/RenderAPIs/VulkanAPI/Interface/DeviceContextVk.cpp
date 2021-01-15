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

        void DeviceContextVk::SetViewports(Uint32 numViewports, const Viewport* pViewports)
        {
        }

        void DeviceContextVk::SetScissors(Uint32 numScissors, const Rect2D* pScissors)
        {
        }
    } // namespace rapi
} // namespace cube
