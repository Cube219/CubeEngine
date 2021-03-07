#include "TextureViewVk.h"

#include "TextureVk.h"
#include "../VulkanDebug.h"
#include "../VulkanDevice.h"
#include "../VulkanTypeConversion.h"

namespace cube
{
    namespace rapi
    {
        TextureViewVk::TextureViewVk(VulkanDevice& device, SPtr<TextureVk>& pTexture, const TextureViewCreateInfo& info) :
            mDevice(device),
            mpTexture(pTexture)
        {
            VkResult res;

            VkImageViewCreateInfo viewCreateInfo = {};
            viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewCreateInfo.pNext = nullptr;
            viewCreateInfo.flags = 0;
            viewCreateInfo.image = pTexture->GetHandle();

            switch(info.dimension) {
                case TextureViewDimension::TextureView1D:        viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_1D;         break;
                case TextureViewDimension::TextureView1DArray:   viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;   break;
                case TextureViewDimension::TextureView2D:        viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;         break;
                case TextureViewDimension::TextureView2DArray:   viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;   break;
                case TextureViewDimension::TextureView3D:        viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;         break;
                case TextureViewDimension::TextureViewCube:      viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;       break;
                case TextureViewDimension::TextureViewCubeArray: viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY; break;
                default:
                    ASSERTION_FAILED("Unknown TextureViewDimension ({})", info.dimension);
                    break;
            }

            viewCreateInfo.format = TextureFormatToVkFormat(info.format);
            viewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
            viewCreateInfo.subresourceRange.baseMipLevel = info.mipmapStartIndex;
            viewCreateInfo.subresourceRange.levelCount = info.numMipmaps;
            if(info.numMipmaps == 0) viewCreateInfo.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;

            if(info.dimension == TextureViewDimension::TextureView1DArray ||
                info.dimension == TextureViewDimension::TextureView2DArray ||
                info.dimension == TextureViewDimension::TextureViewCube ||
                info.dimension == TextureViewDimension::TextureViewCubeArray) {
                viewCreateInfo.subresourceRange.baseArrayLayer = info.arrayStartIndex;
                viewCreateInfo.subresourceRange.layerCount = info.numArrays;
                if(info.numArrays == 0) viewCreateInfo.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
            } else {
                viewCreateInfo.subresourceRange.baseArrayLayer = 0;
                viewCreateInfo.subresourceRange.layerCount = 1;
            }

            viewCreateInfo.subresourceRange.aspectMask = 0;
            if(info.type == TextureViewType::DepthStencilView) {
                viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            } else {
                viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            }

            res = vkCreateImageView(mDevice.GetHandle(), &viewCreateInfo, nullptr, &mImageView);
            CHECK_VK(res, "Failed to create VkImageView.");
            VULKAN_SET_OBJ_NAME(mDevice.GetHandle(), mImageView, info.debugName);
        }

        TextureViewVk::~TextureViewVk()
        {
            vkDestroyImageView(mDevice.GetHandle(), mImageView, nullptr);
        }
    } // namespace rapi
} // namespace cube
