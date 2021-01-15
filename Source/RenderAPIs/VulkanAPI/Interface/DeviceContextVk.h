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

            virtual void SetViewports(Uint32 numViewports, const Viewport* pViewports);

            virtual void SetScissors(Uint32 numScissors, const Rect2D* pScissors);

        private:
            VulkanDevice& mDevice;

            VulkanCommandPool mGraphicsCommandPool;
            VulkanCommandPool mComputeCommandPool;
        };
    }
} // namespace cube
