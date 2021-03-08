#pragma once

#include "../VulkanAPIHeader.h"

#include "RenderAPIs/RenderAPI/Interface/TextureView.h"

namespace cube
{
    namespace rapi
    {
        class TextureViewVk : public TextureView
        {
        public:
            TextureViewVk(VulkanDevice& device, SPtr<TextureVk>& pTexture, const TextureViewCreateInfo& info);
            virtual ~TextureViewVk();

            VkImageView GetHandle() const { return mImageView; }

        private:
            VulkanDevice& mDevice;
            SPtr<TextureVk> mpTexture;

            VkImageView mImageView;
        };
    } // namespace rapi
} // namespace cube
