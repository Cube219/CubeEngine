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
            TextureViewVk(const TextureViewCreateInfo& info);
            virtual ~TextureViewVk();

        private:
            VkImageView mImageView;
        };
    } // namespace rapi
} // namespace cube
