#pragma once

#include "../VulkanAPIHeader.h"

#include "RenderAPIs/RenderAPI/Interface/Framebuffer.h"

namespace cube
{
    namespace rapi
    {
        class FramebufferVk : public Framebuffer
        {
        public:
            FramebufferVk(VulkanDevice& device, const FramebufferCreateInfo& info);
            virtual ~FramebufferVk();

            VkFramebuffer GetHandle() const { return mFramebuffer; }

        private:
            VulkanDevice& mDevice;

            VkFramebuffer mFramebuffer;
        };
    } // namespace rapi
} // namespace cube
