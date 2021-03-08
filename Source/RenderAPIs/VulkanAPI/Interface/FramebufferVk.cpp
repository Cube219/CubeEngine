#include "FramebufferVk.h"

#include "RenderPassVk.h"
#include "TextureViewVk.h"
#include "../VulkanDevice.h"

namespace cube
{
    namespace rapi
    {
        FramebufferVk::FramebufferVk(VulkanDevice& device, const FramebufferCreateInfo& info) :
            mDevice(device)
        {
            VkResult res;
            
            VkRenderPass vkRenderPass = DCast(RenderPassVk*)(info.pRenderPass)->GetVkRenderPass();

            FrameVector<VkImageView> attachments(info.numAttachments);
            for(Uint32 i = 0; i < info.numAttachments; ++i) {
                attachments[i] = DCast(TextureViewVk*)(info.ppAttachments[i])->GetHandle();
            }

            VkFramebufferCreateInfo framebufferCreateInfo;
            framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferCreateInfo.pNext = nullptr;
            framebufferCreateInfo.flags = 0;
            framebufferCreateInfo.renderPass = vkRenderPass;
            framebufferCreateInfo.attachmentCount = info.numAttachments;
            framebufferCreateInfo.pAttachments = attachments.data();
            framebufferCreateInfo.width = info.width;
            framebufferCreateInfo.height = info.height;
            framebufferCreateInfo.layers = info.arrayStartIndex;
            
            res = vkCreateFramebuffer(mDevice.GetHandle(), &framebufferCreateInfo, nullptr, &mFramebuffer);
            CHECK_VK(res, "Failed to create VkFramebuffer.");
        }

        FramebufferVk::~FramebufferVk()
        {
            vkDestroyFramebuffer(mDevice.GetHandle(), mFramebuffer, nullptr);
        }
    } // namespace rapi
} // namespace cube
