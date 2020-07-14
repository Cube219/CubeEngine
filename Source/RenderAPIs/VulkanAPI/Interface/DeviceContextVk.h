#pragma once

#include "../VulkanAPIHeader.h"

#include "RenderAPIs/RenderAPI/Interface/DeviceContext.h"

#include "../VulkanCommandPool.h"

namespace cube
{
    namespace rapi
    {
        class DeviceContextVk : public DeviceContext
        {
        public:
            DeviceContextVk(VulkanDevice& device);
            virtual ~DeviceContextVk();

            VulkanCommandBuffer& GetUploadCommandBuffer() { return mUploadCommandBuffer; }

        private:
            VulkanDevice& mDevice;

            VulkanCommandPool mCommandPool;
            VulkanCommandBuffer mUploadCommandBuffer;
        };
    }
} // namespace cube
