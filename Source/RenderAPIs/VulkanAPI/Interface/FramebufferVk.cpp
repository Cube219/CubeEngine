#include "FramebufferVk.h"

#include "RenderPassVk.h"
#include "../VulkanDevice.h"

namespace cube
{
    namespace rapi
    {
        FramebufferVk::FramebufferVk(VulkanDevice& device, const FramebufferCreateInfo& info) :
            mDevice(device)
        {
            // VkResult res;
            //
            // VkRenderPass vkRenderPass = DCast(RenderPassVk*)(info.pRenderPass)->GetVkRenderPass();
            //
            // VkFramebufferCreateInfo framebufferCreateInfo;
            // framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            // framebufferCreateInfo.pNext = nullptr;
            // framebufferCreateInfo.flags = 0;
            // framebufferCreateInfo.renderPass = vkRenderPass;
            // framebufferCreateInfo.attachmentCount = SCast(Uint32)(attachments.size());
            // framebufferCreateInfo.pAttachments = attachments.data();
            // framebufferCreateInfo.width = width;
            // framebufferCreateInfo.height = height;
            // framebufferCreateInfo.layers = 1;
            //
            // res = vkCreateFramebuffer(mDevice.GetHandle(), &framebufferCreateInfo, nullptr, &mFramebuffer);
            // CHECK_VK(res, "Failed to create VkFramebuffer.");
        }

        FramebufferVk::~FramebufferVk()
        {
            vkDestroyFramebuffer(mDevice.GetHandle(), mFramebuffer, nullptr);
        }
    } // namespace rapi
} // namespace cube
