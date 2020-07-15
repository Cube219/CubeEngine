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

        private:
            VulkanDevice& mDevice;

            VulkanCommandPool mGraphicsCommandPool;
            VulkanCommandPool mComputeCommandPool;
        };
    }
} // namespace cube
