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

            Uint32 GetWidth() const { return mWidth; }
            Uint32 GetHeight() const { return mHeight; }

        private:
            VulkanDevice& mDevice;

            VkFramebuffer mFramebuffer;

            Uint32 mWidth;
            Uint32 mHeight;
        };
    } // namespace rapi
} // namespace cube
