#pragma once

#include "../VulkanAPIHeader.h"

#include "RenderAPIs/RenderAPI/Interface/RenderPass.h"

namespace cube
{
    namespace rapi
    {
        VkAttachmentLoadOp ConvertToVkAttachmentLoadOp(LoadOperator loadOp);
        VkAttachmentStoreOp ConvertToVkAttachmentStoreOp(StoreOperator storeOp);

        class RenderPassVk : public RenderPass
        {
        public:
            RenderPassVk(VulkanDevice& device, const RenderPassCreateInfo& info);
            virtual ~RenderPassVk();

            VkRenderPass GetVkRenderPass() const { return mRenderPass; }

        private:
            VulkanDevice& mDevice;

            VkRenderPass mRenderPass;
        };
    } // namespace rapi
} // namespace cube
